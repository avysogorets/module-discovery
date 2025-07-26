import pandas as pd
import os
import logging
from ..data_base import DataBase
from ..utils.utils import default_collate_fn, DataCollatorForMaskedLM
from ..utils.language import *
from ...globals import PAD, CLS, MASK, EOS, PROJECT_DIR


class LanguageDataBase(DataBase):
    def __init__(self, backbone_path, **kwargs):
        super().__init__(**kwargs)
        self.backbone_path = backbone_path
        self.logger = logging.getLogger(__name__)

    def create_datasets(self, tokenizer, **kwargs):
        data_to_tokenize = {}
        path = os.path.join(f'{PROJECT_DIR}/assets/language', self.dataset_name)
        path_train = os.path.join(path, 'train.tsv')
        path_test = os.path.join(path, 'dev.tsv')
        data_to_tokenize['train'] = pd.read_csv(path_train, sep='\t', index_col=0)
        data_to_tokenize['test'] = pd.read_csv(path_test, sep='\t', index_col=0)
        for split in ['train', 'test']:
            self.datasets[split] = get_dataset_from_df(
                data_to_tokenize[split],
                tokenizer=tokenizer,
                max_length=self.max_length,
                split=split)

    def create_assets(self, **kwargs):
        tokenizer, max_pos_emb = get_tokenizer_and_length(self.backbone_path)
        if self.max_length > max_pos_emb:
            self.logger.warning(f"specified max_length ({self.max_length}) reduced to {max_pos_emb}")
        self.max_length = min(self.max_length, max_pos_emb)
        self.create_datasets(tokenizer)
        self.token_dict = {}
        for token, i in tokenizer.vocab.items():
            self.token_dict[token] = i
        self.special_tokens = {
            PAD: tokenizer.pad_token,
            CLS: tokenizer.cls_token,
            EOS: tokenizer.eos_token,
            MASK: tokenizer.mask_token}
        masked_collator = DataCollatorForMaskedLM(
            mask_id=tokenizer.mask_token_id,
            vocab_size=len(tokenizer),
            mask_ratio=0.15)
        self.collate_fns = {
            'default': default_collate_fn,
            'masked': masked_collator}

