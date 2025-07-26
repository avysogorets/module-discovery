from .language_base import LanguageDataBase


class WebText(LanguageDataBase):
    """ Human (0) vs. GPT-2 XL-1542M (1) generated text.
        Source: https://github.com/openai/gpt-2-output-dataset
    """
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.dataset_name = 'webtext'
        self.num_classes = 2
        self.create_assets()