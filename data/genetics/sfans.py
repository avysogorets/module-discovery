import numpy as np
import logging
from collections import Counter
from .genetics_base import GeneticsDataBase


class sFANS(GeneticsDataBase):
    def __init__(self, max_length, subset_hvg, **model_kwargs):
        super().__init__(max_length=max_length, subset_hvg=subset_hvg)
        self.dataset_name = 'sFANS'
        self.model_name = model_kwargs['model_name']
        self.model_kwargs = model_kwargs
        self.labels_map = {'NC': 0, 'HD': 1, 'AD': 2}
        self.num_classes = 3
        self.label_name = 'diagnosis'
        self.gene_identifier = 'ensembl_id'
        self.logger = logging.getLogger(__name__)
        self.create_assets()

    def get_train_idxs(self, N, **kwargs):
        return list(range(N))

    def _get_train_idxs(self, N, **kwargs):
        donors = np.unique(self.cell_attrs['donor'])
        donor_idxs = {donor: [] for donor in donors}
        donor2diagnosis = {}
        for i in range(N):
            donor = self.cell_attrs['donor'][i]
            diagnosis = self.cell_attrs['diagnosis'][i]
            if donor not in donor2diagnosis:
                donor2diagnosis[donor] = diagnosis
            assert donor2diagnosis[donor] == diagnosis
            donor_idxs[donor].append(i)
        train_diagnoses = []
        test_diagnoses = []
        num_attempts = 0
        while len(set(train_diagnoses))<3 and len(set(test_diagnoses))<3:
            num_attempts += 1
            train_donors = np.random.choice(
                donors,
                size=int(self.train_factor*len(donors)),
                replace=False)
            train_donors = train_donors.tolist()
            test_donors = [donor for donor in donors if donor not in train_donors]
            train_diagnoses = [donor2diagnosis[donor] for donor in train_donors]
            test_diagnoses = [donor2diagnosis[donor] for donor in test_donors]
            assert num_attempts < 100, f"cannot split sFANS (attempts: {num_attempts})"
        for diagnosis, num_donors in Counter(train_diagnoses):
            self.logger.info(f'[sFANS split][train][diagnosis: {num_donors}]')
        for diagnosis, num_donors in Counter(test_diagnoses):
            self.logger.info(f'[sFANS split][test][diagnosis: {num_donors}]')
        train_idxs = []
        for donor in train_donors:
            train_idxs += donor_idxs[donor]
        return train_idxs
    

class sFANS_0530(sFANS):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.dataset_name = 'sFANS_0530'
        self.create_assets()

    def get_train_idxs(self, N, **kwargs):
        return list(range(N))
    

class sFANS_0531(sFANS):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.dataset_name = 'sFANS_0531'
        self.create_assets()

    def get_train_idxs(self, N, **kwargs):
        return list(range(N))


class sFANS_M_NC(sFANS):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.dataset_name = 'sFANS_M_NC'
        self.create_assets()

    def get_train_idxs(self, N, **kwargs):
        return list(range(N))
    

class sFANS_M_AD(sFANS):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.dataset_name = 'sFANS_M_AD'
        self.create_assets()

    def get_train_idxs(self, N, **kwargs):
        return list(range(N))
    

class sFANS_M_HD(sFANS):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.dataset_name = 'sFANS_M_HD'
        self.create_assets()

    def get_train_idxs(self, N, **kwargs):
        return list(range(N))
    

class sFANS_ADHD(sFANS):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.dataset_name = 'sFANS_ADHD'
        self.create_assets()

    def get_train_idxs(self, N, **kwargs):
        return list(range(N))

        


        