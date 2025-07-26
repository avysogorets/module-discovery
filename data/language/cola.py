from .language_base import LanguageDataBase


class CoLa(LanguageDataBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'cola'
        self.num_classes = 2
        self.create_assets()