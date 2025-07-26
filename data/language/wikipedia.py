import logging
from .language_base import LanguageDataBase
from ..utils.utils import ShardedDataset
from ..utils.language import get_dataset_from_ds
from ...globals import PROJECT_DIR


class Wikipedia(LanguageDataBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.supervised = False
        self.allow_perm = False
        self.logger = logging.getLogger(__name__)
        self.create_assets()

    def create_datasets(self, tokenizer, **kwargs):
        path = f'{PROJECT_DIR}/assets/language/wikipedia'
        self.datasets['train'] = ShardedDataset(
            path=path,
            create_dataset_fn=get_dataset_from_ds,
            tokenizer=tokenizer,
            max_length=self.max_length,
            split='train')
        self.datasets['test'] = None

    # def get_dataloader(self, num_workers, *args, **kwargs):
    #     if num_workers > 0:
    #         self.logger.warning(f'ShardedDataset requires 0 workers (provided: {num_workers})')
    #     return super().get_dataloader(*args, num_workers=0, **kwargs)