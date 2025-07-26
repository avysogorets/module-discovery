import torch
from transformers import RobertaModel, RobertaForMaskedLM
from .model_base import ModelBase


class RoBERTa(ModelBase):
    def __init__(self, num_classes, model_kwargs, **kwargs):
        super().__init__(**kwargs)
        self.encoder = RobertaModel.from_pretrained(model_kwargs['backbone_path'], add_pooling_layer=False)
        self.encoder.config._attn_implementation_internal = 'eager'
        self.output_heads['MLM'] = RobertaForMaskedLM(self.encoder.config).lm_head
        if num_classes:
            self.output_heads['Supervised'] = torch.nn.Linear(self.encoder.config.hidden_size, num_classes)
        self.to(self.device)
        self.to(self.dtype)
