from .language_base import LanguageDataBase


class RealNews(LanguageDataBase):
    """ Human (0) vs. GROVER (1) generated news artcles.
        Source: https://github.com/rowanz/grover/tree/master/generation_examples
    """
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'realnews'
        self.num_classes = 2
        self.create_assets()