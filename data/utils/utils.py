import torch
import os
from datasets import Dataset
from tqdm.auto import tqdm


class TokenizedDataset(torch.utils.data.Dataset):
    def __init__(self, inputs, global_idxs=None, split_code=None):
        if global_idxs is None:
            global_idxs = list(range(len(inputs)))
        self.inputs = inputs
        for i in range(len(inputs)):
            self.inputs[i]['local_idx'] = i
            if global_idxs is not None:
                self.inputs[i]['global_idx'] = global_idxs[i]
            if split_code is not None:
                self.inputs[i]['split'] = split_code

    def __len__(self):
        return len(self.inputs)
    
    def __getitem__(self, idx):
        return self.inputs[idx]
    

class ShardedDataset(torch.utils.data.Dataset):
    def __init__(self, path, create_dataset_fn, **create_dataset_kwargs):
        self.path = path
        sfns = sorted(os.listdir(path))
        self.shard_filenames = [sfn for sfn in sfns if sfn.endswith('.arrow')]
        self.total_len = 0
        self.shard_idx = 0
        for _ in self.shard_filenames:
            shard = self.load_shard()
            self.total_len += len(shard)
            self.shard_idx += 1
        self.create_dataset_fn = create_dataset_fn
        self.create_dataset_kwargs = create_dataset_kwargs

    def load_shard(self):
        filename = self.shard_filenames[self.shard_idx]
        filepath = os.path.join(self.path, filename)
        dataset = Dataset.from_file(filepath)
        return dataset
    
    def set_shard(self):
        shard = self.load_shard()
        self.dataset = self.create_dataset_fn(
            shard,
            **self.create_dataset_kwargs)

    def __len__(self):
        return self.total_len
    
    def __getitem__(self, idx):
        if idx == 0:
            self.shard_idx = 0
            self.base_idx = 0
            self.set_shard()
        elif idx >= self.base_idx + len(self.dataset):
            self.base_idx += len(self.dataset)
            self.shard_idx += 1
            self.set_shard()
            err_msg = ("loaded new shard but requested idx still out of bounds...\n",
                "it is likely you iterate over the dataset non-sequentially.")
            assert idx <= self.base_idx + len(self.dataset), err_msg
        return self.dataset[idx-self.base_idx]
        

def default_collate_fn(batch):
    keys = list(batch[0].keys())
    batch_dict = {key: [] for key in keys}
    for datum in batch:
        for key in keys:
            batch_dict[key].append(datum[key])
    for key in keys:
        if key in ['label', 'local_idx', 'global_idx', 'split']:
            if batch_dict[key][-1] is not None:
                batch_key_tensor = torch.LongTensor(batch_dict[key])
                batch_dict[key] = batch_key_tensor
            else:
                batch_dict[key] = None
        else:
            batch_key_tensor = torch.vstack(batch_dict[key])
            batch_dict[key] = batch_key_tensor
    return batch_dict


class DataCollatorForMaskedLM:
    def __init__(self, mask_id, vocab_size, mask_ratio=0.15):
        self.mask_ratio = mask_ratio
        self.mask_id = mask_id
        self.rand_frac = 0.1
        self.same_frac = 0.1
        self.mask_frac = 0.8
        self.vocab_size = vocab_size

    def __call__(self, batch):
        batch = default_collate_fn(batch)
        labels = batch['input_ids'].clone()
        probability_matrix = torch.full(labels.shape, self.mask_ratio)
        special_tokens_mask = batch['spec_masks'].bool()
        probability_matrix.masked_fill_(special_tokens_mask, value=0.0)
        idxs_replaced = torch.bernoulli(probability_matrix).bool()
        labels[~idxs_replaced] = -100
        # replace some tokens with mask token
        idxs_mask = torch.bernoulli(torch.full(labels.shape, self.mask_frac))
        idxs_mask = idxs_mask.bool() & idxs_replaced
        batch['input_ids'][idxs_mask] = self.mask_id
        # replace some tokens with random token
        idxs_rand = torch.bernoulli(torch.full(labels.shape, self.rand_frac))
        idxs_rand = idxs_rand.bool() & idxs_replaced & ~idxs_mask
        tokens_rand = torch.randint(self.vocab_size, labels.shape).long()
        batch['input_ids'][idxs_rand] = tokens_rand[idxs_rand]
        batch['label'] = labels
        return batch
    

def batch_tokenize(texts, tokenizer, batch_size=512, **kwargs):
    all_encodings = []
    for i in tqdm(range(0, len(texts), batch_size)):
        batch = texts[i:i+batch_size]
        encodings = tokenizer.batch_encode_plus(batch, **kwargs) 
        all_encodings.append(encodings)
    inputs = []
    for i in range(len(all_encodings)):
        key = list(all_encodings[i].keys())[0]
        for j in range(len(all_encodings[i][key])):
            inputs.append({
                'input_ids': all_encodings[i]['input_ids'][j],
                'attn_masks': all_encodings[i]['attention_mask'][j],
                'spec_masks': all_encodings[i]['special_tokens_mask'][j]})
    return inputs