import torch
from transformers import BertModel, BertForMaskedLM
from .model_base import ModelBase


class BERT(ModelBase):
    def __init__(self, num_classes, model_kwargs, **kwargs):
        super().__init__(**kwargs)
        self.encoder = BertModel.from_pretrained(
            model_kwargs['backbone_path'],
            add_pooling_layer=False,
            attn_implementation='eager')
        self.encoder.config._attn_implementation_internal = 'eager'
        self.output_heads['MLM'] = BertForMaskedLM(self.encoder.config).cls
        if num_classes:
            self.output_heads['Supervised'] = torch.nn.Linear(
                self.encoder.config.hidden_size,
                num_classes)
        self.to(self.device)
        self.to(self.dtype)
