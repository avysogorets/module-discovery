from .language_base import LanguageDataBase


class Reviews(LanguageDataBase):
    """ Human (0) vs. GPT-2 XL-1542M (1) generated Amazon reviews.
        Source: https://osf.io/tyue9/
    """
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'reviews'
        self.num_classes = 2
        self.create_assets()