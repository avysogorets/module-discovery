import torch
import torch.nn.functional as F
from tqdm.auto import tqdm
from .grapher_base import GrapherBase
from ..globals import ALL, DATASET, MODEL, LAYER, PAD


class Embeddings(GrapherBase):
    def __init__(self, metric, negatives, **kwargs):
        super().__init__(**kwargs)
        self.metric = metric
        self.negatives = negatives
        assert self.collection_level == LAYER
        assert self.aggregation_level in [DATASET, MODEL, LAYER]

    def compute_distance(self, embedding, memory_optimized=False):
        if self.metric == 'L2':
            if memory_optimized:
                assert len(embedding.shape) == 2 # S x D
                D = torch.zeros(embedding.shape[0], embedding.shape[0])
                for i in tqdm(range(embedding.shape[0]), desc='computing distances between embeddings...'):
                    D[i] = (embedding[i][None,:]-embedding).norm(dim=-1)
            else:
                expanded_1 = embedding[...,None,:,:] # ...1 x S x D
                expanded_2 = embedding[...,:,None,:] # ...S x 1 x D
                D = torch.norm(expanded_1-expanded_2, dim=-1, p=2)
            D /= D.amax((-1,-2))[...,None,None]
            D = 1-D
        elif self.metric == 'cosine':
            normalized_1 = F.normalize(embedding, dim=-1, p=2)
            normalized_2 = F.normalize(embedding, dim=-1, p=2)
            expanded_1 = normalized_1[...,None,:,:] # ...1 x S x D
            expanded_2 = normalized_2[...,:,None,:] # ...S x 1 x D
            D = torch.matmul(expanded_2, expanded_1.transpose(-1,-2))
            D = D[...,0,:]
            if self.negatives == 'clamp':
                D = torch.clamp(D, min=0)
            elif self.negatives == 'addmin':
                D -= D.amin((-1,-2))[...,None,None]
                D /= D.amax((-1,-2))[...,None,None]
            elif self.negatives is None:
                pass
            else:
                raise NotImplementedError(f"neg handler {self.negatives} is undefined")
        else:
            raise NotImplementedError(f"metric {self.metric} is undefined")
        return D

    def aggregate2level(self, embedding, **kwargs):
        if self.process_before_aggregation or self.aggregation_level == LAYER:
            tensor = self.compute_distance(embedding) # B x L x S x S
        else:
            tensor = embedding  # B x L x S x D
        if self.aggregation_level in [MODEL, DATASET]:
            if self.layer == ALL:
                tensor = tensor.mean(1)
            else:
                tensor = tensor[:,self.layer,:,:]
        if self.aggregation_level == MODEL and not self.process_before_aggregation:
            tensor = self.compute_distance(tensor)
        return tensor

    def forward(self, model, inp, **kwargs):
        with torch.no_grad():
            embedding,_ = model(
                X=inp,
                output_attentions=False,
                output_hidden_states=True)
            embedding = torch.stack(embedding).permute(1,0,2,3)
            embedding = embedding[:,1:,:,:]
        return embedding

    def __call__(self, model, save_path, split='train', **kwargs):
        self.save_idx = 0
        dataloader = self.data.get_dataloader(
            split=split,
            batch_size=self.max_batch_size,
            num_workers=self.num_workers)
        if dataloader is None:
            self.logger.warning(f'dataloader is None for split {split}')
            return
        T = len(self.data.token_dict)
        hidden_size = model.encoder.config.hidden_size
        tensor_kwargs = {
            'device': self.device,
            'dtype': torch.float32}
        pad_token = self.data.token_dict[self.data.special_tokens[PAD]]
        dataset_distances = torch.zeros(T, T, **tensor_kwargs)
        dataset_embeddings = torch.zeros(T, hidden_size, **tensor_kwargs)
        two_counts = torch.zeros(T, T, **tensor_kwargs)
        one_counts = torch.zeros(T, **tensor_kwargs)
        i = torch.ones(T, **tensor_kwargs)
        I = torch.ones(T, T, **tensor_kwargs)
        num_samples_processed = 0.
        model.eval()
        nan_cnt = 0
        inf_cnt = 0
        for inp in tqdm(dataloader, desc='computing graphs from attention...'):
            if num_samples_processed / len(self.data.datasets[split]) > self.limit_samples_frac:
                break
            for key in inp.keys():
                if key == 'label':
                    continue
                inp[key] = inp[key].to(model.device)
                if 'float' in str(inp[key].dtype):
                    inp[key] = inp[key].to(model.dtype)
            embedding = self.forward(model, inp)
            embedding = self.aggregate2level(embedding)
            embedding = embedding.to(self.device)
            embedding = embedding.to(torch.float32)
            num_samples_processed += embedding.shape[0]
            if embedding.isnan().any().item():
                self.logger.warning(f'NaNs found in attentions in batch: {self.save_idx}')
            if embedding.isinf().any().item():
                self.logger.warning(f'Infs found in attentions in batch: {self.save_idx}')
            if self.aggregation_level != DATASET:
                self.save(A=embedding, filepath=save_path, input_ids=inp['input_ids'])
                continue
            assert len(embedding.shape) == 3, f"shape: {embedding.shape}" # B x S x S or B x S x D
            for b in range(embedding.shape[0]):
                last_idx = torch.where(inp['input_ids'][b] == pad_token)[0]
                if len(last_idx) > 0:
                    last_idx = last_idx[0].item()
                else:
                    last_idx = len(inp['input_ids'][b])
                token_ids = inp['input_ids'][b][:last_idx].cpu()
                local_ids = self.vocab_ids_to_local_ids[token_ids].long()
                if self.process_before_aggregation:
                    tensor = embedding[b, :last_idx, :last_idx]
                else:
                    embd = embedding[b, :last_idx, :]
                local_ids = local_ids.to(self.device)
                if not self.process_before_accumulation and not self.process_before_aggregation:
                    rows = local_ids[:,None].expand(-1, hidden_size)
                    dataset_embeddings.scatter_add_(0, rows, embd)
                    one_counts.scatter_add_(0, local_ids, i[:last_idx])
                    continue
                if self.process_before_accumulation:
                    assert not self.process_before_aggregation
                    tensor = self.compute_distance(embd)
                if tensor.isnan().any().item():
                    nan_cnt += 1
                    continue
                if tensor.isinf().any().item():
                    inf_cnt += 1
                    continue
                rows, cols = torch.meshgrid(local_ids, local_ids, indexing='ij')
                dataset_distances.index_put_((rows, cols), tensor, accumulate=True)
                two_counts.index_put_((rows, cols), I[:last_idx, :last_idx], accumulate=True)
        if self.aggregation_level == DATASET:
            if nan_cnt + inf_cnt > 0:
                self.logger.warning(f'[some skipped due to nans ({nan_cnt}) and infs ({inf_cnt})]')
            existence_flag = (one_counts > 0).int()
            one_counts[one_counts == 0] = 1
            dataset_embeddings /= one_counts[:,None]
            if not self.process_before_accumulation and not self.process_before_aggregation:
                # handle dummy rows for tokens in vocab but not in dataset
                dataset_embeddings[one_counts == 0] = torch.ones(hidden_size)
                dataset_distances = self.compute_distance(dataset_embeddings, memory_optimized=True)
                dataset_distances = dataset_distances*existence_flag[None,:]*existence_flag[:,None]
            self.save(A=dataset_distances, filepath=save_path, two_counts=two_counts)