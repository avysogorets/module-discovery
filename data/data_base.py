import numpy as np
from torch.utils.data import DataLoader
from .utils.utils import TokenizedDataset, default_collate_fn


class DataBase:
    def __init__(self, max_length, **kwargs):
        self.train_factor = 0.8
        self.allow_perm = True
        self.datasets = {
            'train': TokenizedDataset,
            'test': TokenizedDataset}
        self.supervised = True
        self.num_classes = None
        self.token_dict = None
        self.special_tokens = {}
        self.max_length = max_length
        self.collate_fns = {
            'default': default_collate_fn,
            'masked': None}
    
    @property
    def has_test(self):
        if self.datasets['test'] is None:
            return False
        if len(self.datasets['test']) == 0:
            return False
        return True

    def get_train_idxs(self, N, **kwargs):
        assert N > 0, "dataset is empty!"
        train_idxs = np.random.choice(
            range(N),
            size=max(1, int(self.train_factor*N)),
            replace=False).tolist()
        return train_idxs
        
    def create_assets(self, **kwargs):
        """ Populate self.datasets, self.token_dict, self.special_tokens.
        """
        raise NotImplementedError("implement DataBase.create_assets.")

    def get_dataloader(self, batch_size, num_workers, split=None, dataset=None, is_masked=False):
        assert not (split is not None and dataset is not None), "both dataset and split provided"
        if split is None and dataset is None:
            return None
        if dataset is None:
            dataset = self.datasets[split]
        if is_masked:
            collate_fn = self.collate_fns['masked']
        else:
            collate_fn = self.collate_fns['default']
        # do not shuffle; trainer takes care of that
        return DataLoader(
            dataset,
            batch_size=batch_size,
            shuffle=False,
            collate_fn=collate_fn,
            num_workers=num_workers)