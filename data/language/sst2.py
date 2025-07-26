from .language_base import LanguageDataBase


class SST2(LanguageDataBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'sst2'
        self.num_classes = 2
        self.create_assets()