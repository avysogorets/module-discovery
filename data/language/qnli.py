from .language_base import LanguageDataBase


class QNLI(LanguageDataBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'qnli'
        self.num_classes = 2
        self.create_assets()