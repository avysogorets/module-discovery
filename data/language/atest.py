from .language_base import LanguageDataBase
from torch.utils.data import Subset


class ATest(LanguageDataBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'agnews'
        self.num_classes = 4
        self.create_assets()
        self.datasets['train'] = Subset(self.datasets['train'], [0,1])
        self.datasets['test'] = None