import torch
import numpy as np
from ..data_base import DataBase
from ..utils.utils import TokenizedDataset
from ...globals import PAD


class TwoSwitches(DataBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.num_classes = 2
        self.max_length = 6
        self.num_tokens = 6
        self.noise = 0
        self.max_size = 10000
        self.create_datasets()

    def create_datasets(self, **kwargs):
        dataset = []
        groups = [list(range(self.num_tokens//2)),list(range(self.num_tokens//2,self.num_tokens))]
        stack = [[]]
        while len(stack) > 0:
            sample = stack.pop()
            if len(sample) == self.max_length:
                label = int(all((len(set(sample).intersection(set(group)))>0 for group in groups)))
                label = np.random.choice([label,1-label], p=[1-self.noise, self.noise], size=1).item()
                dataset.append({
                    'input_ids': torch.LongTensor(sample),
                    'attn_masks': torch.ones(len(sample)).long(),
                    'label': label})
            else:
                for i in range(self.num_tokens):
                    new_sample = [_ for _ in sample]
                    new_sample.append(i)
                    stack.append(new_sample)
        #idxs_1 = [i for i in range(len(dataset)) if dataset[i]['label'] == 1]
        #idxs_0 = [i for i in range(len(dataset)) if dataset[i]['label'] == 0]
        #idxs_0 = np.random.permutation(idxs_0).tolist()[:self.max_size//2]
        #idxs_1 = np.random.permutation(idxs_1).tolist()[:len(idxs_0)]
        #data_list = [dataset[i] for i in idxs_0+idxs_1]
        data_list = np.random.permutation(dataset)[:self.max_size].tolist()
        train_idxs = self.get_train_idxs(data_list)
        test_idxs = [i for i in range(len(data_list)) if i not in train_idxs]
        train_data_list = [data_list[i] for i in train_idxs]
        test_data_list = [data_list[i] for i in test_idxs]
        self.datasets = {
            'train': TokenizedDataset(train_data_list),
            'test': TokenizedDataset(test_data_list)}
        self.token_dict = {i:i for i in range(self.num_tokens)}
        self.token_dict['<pad>'] = self.num_tokens
        self.special_tokens = {PAD: '<pad>'}