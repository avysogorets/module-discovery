from .language_base import LanguageDataBase


class IMDb(LanguageDataBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'imdb'
        self.num_classes = 2
        self.create_assets()