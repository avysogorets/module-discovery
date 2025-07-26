import numpy as np
import torch
import torch.nn.functional as F
from typing import Tuple
from transformers import BertConfig, BertModel
from .model_base import ModelBase
from ..globals import PAD, CLS


class Tiny(ModelBase):
    def __init__(self, data, num_layers, **kwargs):
        super().__init__(**kwargs)
        hidden_size = 4
        intermediate_size = 8
        self.cls_pool = CLS in data.special_tokens
        config = BertConfig(
            hidden_size=hidden_size,
            intermediate_size=intermediate_size,
            max_position_embeddings=data.max_length+int(self.cls_pool),
            vocab_size=len(data.token_dict),
            hidden_dropout_prob=0,
            attention_probs_dropout_prob=0,
            pad_token_id=data.token_dict[data.special_tokens[PAD]],
            num_attention_heads=1,
            num_hidden_layers=num_layers)
        config._attn_implementation_internal = 'eager'
        self.encoder = BertModel(config, add_pooling_layer=False)
        shape = self.encoder.embeddings.position_embeddings.weight.data.shape
        new_data = torch.zeros(shape).to(self.device)
        self.encoder.embeddings.position_embeddings.weight.data = new_data
        self.encoder.embeddings.position_embeddings.requires_grad_ = False
        frac = np.mean([datum['label'] for datum in data.datasets['train']])
        self.weight = torch.Tensor([frac, 1-frac]).to(self.device)
        self.output_heads = torch.nn.ModuleDict({
            'MLM': None,
            'Supervised': torch.nn.Linear(hidden_size, data.num_classes)})
        self.to(self.device)
        self.to(self.dtype)

    def forward(self, X, **kwargs):
        y = self.encoder(
            X["input_ids"],
            attention_mask=X["attn_masks"],
            token_type_ids=None,
            **kwargs)
        if self.output_type is None:
            if isinstance(y, Tuple):
                return y
            else:
                attentions = None
                if 'attentions' in y.keys():
                    attentions = y["attentions"]
                embeddings = None
                if 'hidden_states' in y.keys():
                    embeddings = y['hidden_states']
                return embeddings, attentions
        elif self.output_type == 'Supervised':
            if self.cls_pool:
                y = y["last_hidden_state"][:,0,:]
            else:
                y = y["last_hidden_state"].mean(1)
            y = self.output_heads['Supervised'](y)
            loss = F.cross_entropy(y, X['label'], weight=self.weight)
            return y, loss
        else:
            raise NotImplementedError(f'unknown head type {self.output_type}')