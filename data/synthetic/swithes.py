import torch
import numpy as np
from ..data_base import DataBase
from ..utils.utils import TokenizedDataset
from ...globals import PAD, CLS


class Switches(DataBase):
    def __init__(self, num_tokens, num_groups, add_cls, **kwargs):
        super().__init__(**kwargs)
        self.num_classes = 2
        self.num_tokens = num_tokens
        self.num_groups = num_groups
        self.token_dict = {i:i for i in range(self.num_tokens)}
        if add_cls:
            self.special_tokens[CLS] = '<cls>'
            self.token_dict[self.special_tokens[CLS]] = len(self.token_dict)
        self.special_tokens[PAD] = '<pad>'
        self.token_dict[self.special_tokens[PAD]] = len(self.token_dict)
        self.add_cls = add_cls
        self.N = 50000
        self.create_datasets()

    def create_datasets(self, **kwargs):
        total = self.num_tokens**self.max_length
        assert self.num_tokens % self.num_groups == 0
        if self.N > total // 3:
            raise RuntimeError(f'N is too large; total datapoints is {total}')
        groups = [g.tolist() for g in np.split(np.arange(self.num_tokens), self.num_groups)]
        dataset = []
        seen = set()
        while len(dataset) < self.N:
            datapoint = np.random.choice(range(self.num_tokens), size=self.max_length, replace=True)
            datapoint = datapoint.tolist()
            if tuple(datapoint) in seen:
                continue
            seen.add(tuple(datapoint))
            label = 1
            for group in groups:
                if len(set(datapoint).intersection(set(group))) == 0:
                    label = 0
            if self.add_cls:
                datapoint = [self.token_dict[self.special_tokens[CLS]]] + datapoint
            dataset.append({
                'input_ids': torch.LongTensor(datapoint),
                'attn_masks': torch.ones(len(datapoint)).long(),
                'label': label})
        data_list = np.random.permutation(dataset)
        train_idxs = self.get_train_idxs(len(data_list))
        test_idxs = [i for i in range(len(data_list)) if i not in train_idxs]
        train_data_list = [data_list[i] for i in train_idxs]
        test_data_list = [data_list[i] for i in test_idxs]
        self.datasets = {
            'train': TokenizedDataset(train_data_list),
            'test': TokenizedDataset(test_data_list)}