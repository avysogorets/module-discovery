import torch
from tqdm.auto import tqdm
from .grapher_base import GrapherBase
from ..globals import DATASET, PAD


class TwoCounts(GrapherBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        assert self.aggregation_level == DATASET
        assert self.collection_level == DATASET
        assert not self.process_before_accumulation
        assert not self.process_before_aggregation
        assert not self.remove_cls
        assert not self.remove_eye
        self.two_counts = None

    def __call__(self, save_path, split='train', **kwargs):
        self.save_idx = 0
        dataloader = self.data.get_dataloader(
            split=split,
            batch_size=self.max_batch_size,
            num_workers=self.num_workers)
        T = len(self.data.token_dict)
        num_samples_processed = 0.
        tensor_kwargs = {
            'device': self.device,
            'dtype': torch.float32}
        pad_token = self.data.token_dict[self.data.special_tokens[PAD]]
        self.two_counts = torch.zeros(T, T, **tensor_kwargs)
        I = torch.ones(T, T, **tensor_kwargs)
        for inp in tqdm(dataloader, desc='computing graphs from attention...'):
            if num_samples_processed / len(self.data.datasets[split]) > self.limit_samples_frac:
                break
            num_samples_processed += inp['input_ids'].shape[0]
            for b in range(inp['input_ids'].shape[0]):
                last_idx = torch.where(inp['input_ids'][b] == pad_token)[0]
                if len(last_idx) > 0:
                    last_idx = last_idx[0].item()
                else:
                    last_idx = len(inp['input_ids'][b])
                token_ids = inp['input_ids'][b][:last_idx]
                local_ids = self.vocab_ids_to_local_ids[token_ids]
                local_ids = local_ids.to(self.device)
                rows, cols = torch.meshgrid(local_ids, local_ids, indexing='ij')
                self.two_counts.index_put_((rows, cols), I[:last_idx, :last_idx], accumulate=True)
        if save_path is not None:
            self.save(A=self.two_counts, filepath=save_path, two_counts=self.two_counts)

