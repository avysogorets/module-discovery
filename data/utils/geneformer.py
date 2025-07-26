import loompy
import numpy as np
import torch
import logging
import pickle
from .utils import default_collate_fn, DataCollatorForMaskedLM
from ...models.geneformer import special_tokens
from ...globals import *


def pad(tokens, pad_id, max_length):
    N = len(tokens)
    pad_tensor = torch.full((max_length-N,), pad_id)
    tokens = torch.cat([tokens, pad_tensor])
    mask = torch.LongTensor(N*[1] + (max_length-N)*[0])
    return tokens, mask


def get_dataset4geneformer(gene_identifier, loom_filepath, max_length, model_kwargs, labels_map=None, label_name=None):
    logger = logging.getLogger(__name__)
    if max_length > model_kwargs['max_seq_len']:
        logger.warning(f"specified max length ({max_length}) reduced to {model_kwargs['max_seq_len']}")
    max_length = min(max_length, model_kwargs['max_seq_len'])
    assert gene_identifier == 'ensembl_id', "gene identifier conversion is not implemented for GeneFormer"
    with open(model_kwargs['token_dict_filepath'], "rb") as f:
        gene_token_dict = pickle.load(f)
    with open(model_kwargs['gene_median_filepath'], "rb") as f:
        gene_median_dict = pickle.load(f)
    gene_keys = list(gene_token_dict.keys())
    genelist_dict = dict(zip(gene_keys, [True]*len(gene_keys)))
    with loompy.connect(loom_filepath, mode='r') as data:
        coding_miRNA_loc = []
        for i in data.ra["ensembl_id"]:
            coding_miRNA_loc.append(genelist_dict.get(i, False))
        coding_miRNA_loc = np.where(coding_miRNA_loc)[0]
        norm_factor_vector = []
        for i in data.ra["ensembl_id"][coding_miRNA_loc]:
            norm_factor_vector.append(gene_median_dict[i])
        norm_factor_vector = np.array(norm_factor_vector)
        coding_miRNA_ids = data.ra["ensembl_id"][coding_miRNA_loc]
        coding_miRNA_tokens = []
        for i in coding_miRNA_ids:
            coding_miRNA_tokens.append(gene_token_dict[i])
        coding_miRNA_tokens = np.array(coding_miRNA_tokens)
        filter_pass_loc = np.arange(data.shape[1])
        dataset = []
        cell_attrs = {attr: [] for attr in data.ca.keys()}
        cell_attrs['exprs'] = []
        special_ids = []
        for id in special_tokens.values():
            if id in gene_token_dict:
                special_ids.append(gene_token_dict[id])
        special_ids = torch.Tensor(special_ids)
        for _,_,view in data.scan(items=filter_pass_loc, axis=1, batch_size=16):
            subview = view.view[coding_miRNA_loc, :]
            subview_norm_array = subview[:, :]
            subview_norm_array = subview_norm_array / subview.ca.n_counts
            subview_norm_array = subview_norm_array * 10000
            for i in range(subview_norm_array.shape[1]):
                cell_attrs['exprs'].append(subview_norm_array[:,i].tolist())
            subview_norm_array = subview_norm_array / norm_factor_vector[:, None]
            for i in range(subview_norm_array.shape[1]):
                gene_vector = subview_norm_array[:,i]
                nonzero_mask = np.nonzero(gene_vector)[0]
                gene_vector = gene_vector[nonzero_mask]
                gene_tokens = coding_miRNA_tokens[nonzero_mask]
                sorted_indices = np.argsort(-gene_vector)
                if model_kwargs['model_size'] == '30M':
                    # note that the official GeneFormer code does not include CLS for 30M-series.
                    cell_tokens = gene_tokens[sorted_indices][:max_length]
                elif model_kwargs['model_size']  == '95M':
                    cell_tokens = gene_tokens[sorted_indices][:max_length-2]
                    cell_tokens = np.insert(cell_tokens, 0, gene_token_dict.get(special_tokens[CLS]))
                    N = len(cell_tokens)
                    cell_tokens = np.insert(cell_tokens, N, gene_token_dict.get(special_tokens[EOS]))
                else:
                    raise NotImplementedError(f"unknown MODEL_SIZE: {model_kwargs['model_size'] }")
                cell_tokens = torch.from_numpy(cell_tokens)
                cell_tokens, attn_mask = pad(cell_tokens,
                        gene_token_dict.get(special_tokens[PAD]),
                        max_length=max_length)
                spec_idxs = torch.where(torch.isin(cell_tokens, special_ids))[0]
                spec_mask = torch.zeros(len(cell_tokens))
                spec_mask[spec_idxs] = 1.
                dataset.append({
                    "input_ids": cell_tokens.long(),
                    "label": labels_map[subview.ca[label_name][i]] if label_name else None,
                    "attn_masks": attn_mask.long(),
                    "spec_masks": spec_mask.long()})
                for key in data.ca.keys():
                    cell_attrs[key].append(data.ca[key][i])
    assert len(coding_miRNA_tokens) == len(cell_attrs['exprs'][0])
    if label_name:
        del cell_attrs[label_name]
    masked_collator = DataCollatorForMaskedLM(
        mask_id=gene_token_dict[special_tokens[MASK]],
        vocab_size=len(gene_token_dict),
        mask_ratio=0.15)
    collate_fns = {
        'default' : default_collate_fn,
        'masked': masked_collator}
    return dataset, gene_token_dict, special_tokens, cell_attrs, coding_miRNA_tokens, collate_fns