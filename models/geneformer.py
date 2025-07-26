import torch
from transformers import BertForTokenClassification
from transformers.models.bert.modeling_bert import BertOnlyMLMHead
from .model_base import ModelBase
from ..globals import *


special_tokens = {
    PAD: '<pad>',
    EOS: '<eos>',
    CLS: '<cls>',
    MASK: '<mask>'}


class GeneFormer(ModelBase):
    def __init__(self, model_kwargs, num_classes, **kwargs):
        super().__init__(**kwargs)
        encoder_config = model_kwargs['encoder_config']
        self.encoder = BertForTokenClassification.from_pretrained(**encoder_config).bert
        self.output_heads['MLM'] = BertOnlyMLMHead(self.encoder.config)
        if num_classes:
            self.output_heads['Supervised'] = torch.nn.Linear(
                self.encoder.config.hidden_size,
                num_classes)
        self.to(self.device)
        self.to(self.dtype)