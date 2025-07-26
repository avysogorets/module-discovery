import logging
import torch
import os
from tqdm.auto import tqdm
from .utils import mask_pad, expand_mask
from ..utils import compute_correlations, create_ids_mapping
from ..globals import *


class GrapherBase:
    def __init__(self,
            data,
            device, 
            max_batch_size, 
            num_workers, 
            aggregation_level,
            collection_level, 
            process_before_aggregation,
            process_before_accumulation,
            limit_samples_frac=1.0,
            negatives=None,
            remove_eye=False,
            remove_cls=False,
            layer=ALL,
            **kwargs):
        self.data = data
        self.device = device
        self.process_before_aggregation = process_before_aggregation
        self.process_before_accumulation = process_before_accumulation
        self.limit_samples_frac = limit_samples_frac
        self.remove_eye = remove_eye
        self.remove_cls = remove_cls
        self.negatives = negatives
        self.aggregation_level = aggregation_level
        self.collection_level = collection_level
        self.max_batch_size = max_batch_size
        self.num_workers = num_workers
        self.layer = layer
        self.save_idx = 0
        self.vocab_ids_to_local_ids = create_ids_mapping(data)
        self.logger = logging.getLogger(__name__)

    def save(self, A, filepath, input_ids=None, two_counts=None, **kwargs):
        A = A.to_sparse()
        torch.save(A, os.path.join(filepath, f'attentions_{self.save_idx}.pt'))
        if input_ids is not None:
            torch.save(input_ids, os.path.join(filepath, f'input_ids_{self.save_idx}.pt'))
        if two_counts is not None:
            two_counts = two_counts.to_sparse()
            torch.save(two_counts, os.path.join(filepath, f'two_counts_{self.save_idx}.pt'))
            vocab_ids_to_local_ids = self.vocab_ids_to_local_ids.to_sparse()
            torch.save(vocab_ids_to_local_ids, os.path.join(
                filepath,
                f'vocab_ids_to_local_ids_{self.save_idx}.pt'))
        self.save_idx += 1

    def aggregate2level(self, attn, masks=None, **kwargs):
        """ Input: attention from running forward, 2D binary mask (optional)
            Output: aggregated attention according to aggregation level
            - HEAD:    B x L x H x S x S
            - LAYER:   B x L x S x S
            - MODEL:   B x S x S
            - DATASET: B x S x S (__call__ logic will reduce to V x V)
        """
        if masks is not None:
            masks = expand_mask(attn, masks)
        if self.process_before_aggregation:
            attn = compute_correlations(
                attn,
                masks=masks,
                remove_eye=self.remove_eye,
                remove_cls=self.remove_cls)
        if self.collection_level == HEAD:
            assert len(attn.shape) == 5
            if self.aggregation_level < HEAD:
                attn = attn.sum(2)
            if self.aggregation_level < LAYER:
                if self.layer == ALL:
                    attn = attn.mean(1)
                else:
                    attn = attn[:,self.layer,...]
        if self.collection_level == LAYER:
            # TODO: edge case when shape has 5 
            # but collection is 2 (e.g., Raw)
            assert len(attn.shape) >= 4
            if self.aggregation_level < LAYER:
                if self.layer == ALL:
                    attn = attn.mean(1)
                else:
                    attn = attn[:,self.layer,...]
        if self.collection_level in [MODEL, DATASET]:
            # TODO: edge case when shape has 5 
            # but collection is 1 (e.g., Raw)
            assert len(attn.shape) == 4
            assert attn.shape[1] == 1
            attn = attn[:,0,:,:]
        return attn
    
    def forward(self, model, inp, **kwargs):
        with torch.no_grad():
            _,attention = model(
                X=inp,
                output_attentions=True,
                output_hidden_states=False)
        attention = torch.stack(attention).permute(1,0,2,3,4)
        return attention

    def __call__(self, model, save_path, split='train', **kwargs):
        self.save_idx = 0
        dataloader = self.data.get_dataloader(
            split=split,
            batch_size=self.max_batch_size,
            num_workers=self.num_workers)
        if dataloader is None:
            self.logger.warning(f'dataloader is None for split {split}')
            return
        T = len(self.vocab_ids_to_local_ids)
        if isinstance(model, torch.nn.DataParallel):
            model_device = model.module.device
            model_dtype = model.module.dtype
        else:
            model_device = model.device
            model_dtype = model.dtype
        tensor_kwargs = {
            'device': self.device,
            'dtype': torch.float32}
        pad_token = self.data.token_dict[self.data.special_tokens[PAD]]
        dataset_attentions = torch.zeros(T, T, **tensor_kwargs)
        two_counts = torch.zeros(T, T, **tensor_kwargs)
        I = torch.ones(T, T, **tensor_kwargs)
        num_samples_processed = 0.
        model.eval()
        nan_cnt = 0
        inf_cnt = 0
        for inp in tqdm(dataloader, desc='computing graphs from attention...'):
            if num_samples_processed / len(self.data.datasets[split]) > self.limit_samples_frac:
                self.logger.info(f'processed {num_samples_processed} samples, which is the limit.')
                break
            for key in inp.keys():
                if key == 'label':
                    continue
                inp[key] = inp[key].to(model_device)
                if 'float' in str(inp[key].dtype):
                    inp[key] = inp[key].to(model_dtype)
            attention = self.forward(model, inp)
            # I decided to mask attention to padding tokens straight away
            batch_masks = inp['attn_masks'].to(attention.device)
            attention = mask_pad(attention, batch_masks)
            attention = self.aggregate2level(attention, masks=batch_masks)
            attention = attention.to(self.device)
            attention = attention.to(torch.float32)
            num_samples_processed += attention.shape[0]
            if attention.isnan().any().item():
                self.logger.warning(f'NaNs found in attentions in batch: {self.save_idx}')
            if attention.isinf().any().item():
                self.logger.warning(f'Infs found in attentions in batch: {self.save_idx}')
            if self.aggregation_level != DATASET:
                self.save(A=attention, filepath=save_path, input_ids=inp['input_ids'])
                continue
            assert len(attention.shape) == 3 # B x S x S
            for b in range(attention.shape[0]):
                last_idx = torch.where(inp['input_ids'][b] == pad_token)[0]
                if len(last_idx) > 0:
                    last_idx = last_idx[0].item()
                else:
                    last_idx = len(inp['input_ids'][b])
                token_ids = inp['input_ids'][b][:last_idx]
                attn = attention[b, :last_idx, :last_idx]
                if self.process_before_accumulation:
                    assert not self.process_before_aggregation
                    attn = compute_correlations(
                        attn,
                        remove_eye=self.remove_eye,
                        remove_cls=self.remove_cls)
                if attn.isnan().any().item():
                    nan_cnt += 1
                    continue
                if attn.isinf().any().item():
                    inf_cnt += 1
                    continue
                token_ids = token_ids.to(self.device)
                local_ids = self.vocab_ids_to_local_ids[token_ids]
                rows, cols = torch.meshgrid(local_ids, local_ids, indexing='ij')
                dataset_attentions.index_put_((rows, cols), attn, accumulate=True)
                two_counts.index_put_((rows, cols), I[:last_idx, :last_idx], accumulate=True)
        if self.aggregation_level == DATASET:
            if nan_cnt + inf_cnt > 0:
                self.logger.warning(f'[some skipped due to nans ({nan_cnt}) and infs ({inf_cnt})]')
            self.save(A=dataset_attentions, filepath=save_path, two_counts=two_counts)