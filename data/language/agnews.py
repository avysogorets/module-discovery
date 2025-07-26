from .language_base import LanguageDataBase


class AGNews(LanguageDataBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'agnews'
        self.num_classes = 4
        self.create_assets()