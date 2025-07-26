import json
import pandas as pd
import numpy as np
import os
import torch
import loompy
import anndata
import logging
from typing import List, Dict
from scgpt.tokenizer import GeneVocab, tokenize_batch
from scipy.sparse import issparse
from ...external.scgpt.preprocess import Preprocessor
from ...globals import *
from ...models.scgpt import special_tokens
from ...utils import get_name2id_id2name


class scGPTDataCollator:
    """ Based on scgpt.data_collator.DataCollator"""
    def __init__(self,
            max_length,
            pad_token_id,
            special_ids,
            exprs,
            gene_ids,
            to_sample=False,
            to_mask=False,
            to_order=True,
            pad_value=0,
            mask_value=-1,
            mask_ratio=0.15,
            keep_first_n_tokens=1):
        self.max_length = max_length
        self.mask_value = mask_value
        self.mask_ratio = mask_ratio
        self.special_ids = special_ids
        self.pad_token_id = pad_token_id
        self.pad_value = pad_value
        self.to_sample = to_sample
        self.to_mask = to_mask
        self.to_order = to_order
        self.exprs = exprs
        self.gene_ids = gene_ids
        self.keep_first_n_tokens = keep_first_n_tokens
        self.id2idx = {}
        for idx,id in enumerate(self.gene_ids):
            self.id2idx[id] = idx
    
    def __call__(self, batch: List[Dict[str, torch.Tensor]]):
        """ expects a batch of dictionaries, samples tokenized, binned, unpadded. 
        """
        batch_input_ids = []
        batch_values = []
        batch_labels = []
        batch_spec_masks = []
        batch_attn_masks = []
        batch_local_idxs = []
        for i in range(len(batch)):
            input_ids = batch[i]['input_ids']
            values = batch[i]['values']
            batch_labels.append(batch[i]['label'])
            batch_local_idxs.append(batch[i]['local_idx'])
            device = input_ids.device
            # sampling (not yet sampled if MLM)
            if self.to_sample:
                idxs = torch.randperm(
                    len(input_ids)-self.keep_first_n_tokens,
                    device=device)
                idxs = idxs[:self.max_length-self.keep_first_n_tokens]
                if self.keep_first_n_tokens > 0:
                    idxs = torch.cat([
                        torch.arange(self.keep_first_n_tokens),
                        idxs+self.keep_first_n_tokens], dim=0)
                input_ids = input_ids[idxs]
                values = values[idxs]
            # truncating + padding to specified max length
            if self.to_order:
                argsort = sorted(
                    range(self.keep_first_n_tokens, len(input_ids)),
                    key=lambda j: self.exprs[batch[i]['global_idx']][self.id2idx[input_ids[j].item()]],
                    reverse=True)
                argsort = list(range(self.keep_first_n_tokens)) + argsort
            else:
                argsort = list(range(len(input_ids)))
            input_ids = input_ids[argsort][:self.max_length]
            values = values[argsort][:self.max_length]
            pad_tensor = torch.full((self.max_length-len(input_ids),), self.pad_token_id)
            pad_tensor = pad_tensor.to(device)
            input_ids = torch.cat([input_ids, pad_tensor])
            pad_tensor = torch.full((self.max_length-len(values),), self.pad_value)
            pad_tensor = pad_tensor.to(device)
            values = torch.cat([values, pad_tensor])
            # computing attn / spec masks
            spec_mask = torch.zeros(len(input_ids))
            spec_idxs = torch.where(torch.isin(input_ids, self.special_ids))[0]
            spec_mask[spec_idxs] = 1.
            attn_mask = 1-input_ids.eq(self.pad_token_id).float()
            # attn masks are inverted from the original code because
            # the transformer library is used for models, not torch
            batch_spec_masks.append(spec_mask)
            batch_attn_masks.append(attn_mask)
            batch_input_ids.append(input_ids)
            batch_values.append(values)
        batch_input_ids = torch.stack(batch_input_ids, dim=0)
        batch_input_ids = batch_input_ids.to(device)
        batch_values = torch.stack(batch_values, dim=0)
        batch_values = batch_values.to(device)
        batch_attn_masks = torch.stack(batch_attn_masks, dim=0)
        batch_attn_masks.to(device)
        batch_spec_masks = torch.stack(batch_spec_masks, dim=0)
        batch_spec_masks.to(device)     
        batch_dict = {
            "input_ids": batch_input_ids,
            "values": batch_values,
            "attn_masks": batch_attn_masks,
            "spec_masks": batch_spec_masks,
            "local_idx": torch.LongTensor(batch_local_idxs)}
        if batch_labels[-1] is not None:
            batch_labels = torch.LongTensor(batch_labels).to(device)
            batch_dict['label'] = batch_labels
        # masking of some values (for MLM)
        if self.to_mask:
            batch_labels = batch_values.clone()
            probability_matrix = torch.full(batch_values.shape, self.mask_ratio)
            probability_matrix[batch_values.eq(self.pad_value)] = 0
            if self.keep_first_n_tokens > 0:
                probability_matrix[:,:self.keep_first_n_tokens] = 0
            idxs_mask = torch.bernoulli(probability_matrix).bool().to(device)
            masked_batch_values = batch_values.masked_fill(idxs_mask, self.mask_value)
            batch_dict["values"] = masked_batch_values
            batch_labels[~idxs_mask] = -100
            batch_dict["label"] = batch_labels
        return batch_dict


def loom2anndata(loom_file_path):
    anns_path = os.path.join(PROJECT_DIR, 'assets/genetics/anns')
    dataset_name = loom_file_path.split('/')[-1].rstrip('.loom')
    anns_filepath = os.path.join(anns_path, '.'.join([dataset_name, 'h5ad']))
    if dataset_name in os.listdir(anns_path):
        adata = anndata.read(anns_filepath)
        return adata
    with loompy.connect(loom_file_path, mode='r') as data:
        gene_attrs = list(data.ra.keys())
        cell_attrs = list(data.ca.keys())
        gene_dict = {
            **{attr: data.ra[attr].tolist() for attr in gene_attrs},
            **{'gene_id': [str(j) for j in range(0, data.shape[0])]}}
        cell_dict = {
            **{attr: data.ca[attr].tolist() for attr in cell_attrs},
            **{'cell_id': [str(j) for j in range(0, data.shape[1])]}}
        var_df = pd.DataFrame(gene_dict)
        var_df.set_index('gene_id', inplace=True)
        obs_df = pd.DataFrame(cell_dict)
        obs_df.set_index('cell_id', inplace=True)
        adata = anndata.AnnData(X=data[:,:].T, obs=obs_df, var=var_df)
    adata.write(anns_filepath)


def get_dataset4scgpt(adata, gene_identifier, model_path, max_length, subset_hvg, labels_map=None, label_name=None):
    model_config_file = open(os.path.join(model_path, 'args.json'), 'r')
    model_kwargs = json.load(model_config_file)
    model_config_file.close()
    data_is_raw = True
    logger = logging.getLogger(__name__)
    if max_length > model_kwargs['max_seq_len']:
        logger.warning(f"specified max length ({max_length}) reduced to {model_kwargs['max_seq_len']}")
    max_length = min(max_length, model_kwargs['max_seq_len'])
    mask_value = model_kwargs['mask_value']
    pad_value = model_kwargs['pad_value']
    name2id, id2name = get_name2id_id2name()
    vocab = GeneVocab.from_file(os.path.join(model_path, 'vocab.json'))
    for s in special_tokens.values():
        if s not in vocab:
            vocab.append_token(s)
    if gene_identifier == 'ensembl_id':
        adata.var["gene_name"] = adata.var["ensembl_id"].apply(lambda ens_id: id2name.get(ens_id))
    idxs = []
    for gene in adata.var["gene_name"]:
        if gene and gene in vocab:
            idxs.append(1)
        else:
            idxs.append(-1)
    adata.var["id_in_vocab"] = idxs
    adata = adata[:, adata.var["id_in_vocab"] >= 0]
    preprocessor = Preprocessor(
        use_key="X",
        filter_gene_by_counts=False,
        filter_cell_by_counts=False,
        normalize_total=1e4,
        result_normed_key="X_normed",
        log1p=data_is_raw,
        result_log1p_key="X_log1p",
        subset_hvg=subset_hvg, # added on 05/15/25 and affects all prior experiments with scGPT
        hvg_flavor="seurat_v3" if data_is_raw else "cell_ranger",
        binning=model_kwargs['n_bins'],
        result_binned_key="X_binned")
    preprocessor(adata, batch_key=None)
    input_layer_key = "X_binned"
    cell_attrs = {attr: [] for attr in adata.obs.keys()}
    for key in adata.obs.keys():
        cell_attrs[key] = adata.obs[key].values.tolist()
    if issparse(adata.layers[input_layer_key]):
        all_counts = adata.layers[input_layer_key].A
        raw_counts = adata.layers["X_normed"].A
    else:
        all_counts = adata.layers[input_layer_key]
        raw_counts = adata.layers["X_normed"]
    cell_attrs['exprs'] = [expr.tolist() for expr in raw_counts]
    gene_names = adata.var["gene_name"].tolist()
    vocab.set_default_index(vocab[special_tokens[PAD]])
    gene_ids = np.array(vocab(gene_names), dtype=int)
    tokenized_data = tokenize_batch(
        data=all_counts,
        gene_ids=gene_ids,
        cls_id=vocab[special_tokens[CLS]],
        append_cls=True,
        include_zero_gene=False)
    input_ids = [datum[0] for datum in tokenized_data]
    values = [datum[1] for datum in tokenized_data]
    if label_name:
        labels = adata.obs[label_name].tolist()
    else:
        labels = None
    gene_names = set(gene_names)
    token_dict = {}
    for name, i in vocab.get_stoi().items():
        if name in gene_names:
            token_dict[name2id[name]] = i
        if name in special_tokens.values():
            token_dict[name] = i
    dataset = []
    for i in range(len(input_ids)):
        dataset.append({
            'input_ids': input_ids[i],
            'values': values[i],
            'attn_masks': None, # to be added by collator
            'spec_masks': None, # to be added by collator
            'label': labels_map[labels[i]] if label_name else None})
    # if label_name:
    #    del cell_attrs[label_name]
    special_ids = torch.Tensor([token_dict[id] for id in special_tokens.values()])
    gene_ids = gene_ids.tolist()
    masked_collator = scGPTDataCollator(
        to_sample=True,
        to_mask=True,
        to_order=False, # added on 05/09/25 and affects all prior experiments with scGPT
        exprs=cell_attrs['exprs'],
        gene_ids=gene_ids,
        max_length=max_length,
        pad_token_id=vocab[special_tokens[PAD]],
        pad_value=pad_value,
        mask_value=mask_value,
        mask_ratio=0.25,
        keep_first_n_tokens=1,
        special_ids=special_ids)
    default_collator = scGPTDataCollator(
        to_sample=False,
        to_mask=False,
        to_order=True, # added on 05/09/25 and affects all prior experiments with scGPT
        exprs=cell_attrs['exprs'],
        gene_ids=gene_ids,
        max_length=max_length,
        pad_token_id=vocab[special_tokens[PAD]],
        pad_value=pad_value,
        keep_first_n_tokens=1,
        special_ids=special_ids)
    collators = {
        'default': default_collator,
        'masked': masked_collator}
    return dataset, token_dict, special_tokens, cell_attrs, gene_ids, collators
    
    
    