import torch
import torch.nn.functional as F
from typing import Tuple
from transformers import AutoModelForCausalLM, AutoTokenizer
from .model_base import ModelBase


class GPT2(ModelBase):
    def __init__(self, num_classes, model_kwargs, **kwargs):
        super().__init__(**kwargs)
        self.encoder = AutoModelForCausalLM.from_pretrained(
            model_kwargs['backbone_path'],
            attn_implementation='eager').transformer
        self.encoder.config._attn_implementation_internal = 'eager'
        self.output_heads['MLM'] = AutoModelForCausalLM.from_pretrained(
            model_kwargs['backbone_path'],
            attn_implementation='eager').lm_head
        if num_classes:
            self.output_heads['Supervised'] = torch.nn.Linear(
                self.encoder.config.hidden_size,
                num_classes)
        self.to(self.device)
        self.to(self.dtype)

    @property
    def decompx(self):
        raise NotImplementedError("DecompX is not supported by GPT2")

    def forward(self, X, **kwargs):
        """ This model does not have the CLS token,
            so we implement mean pooling.
        """
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
                    embeddings = y["hidden_states"]
                return embeddings, attentions
        elif self.output_type == 'MLM':
            y = y["last_hidden_state"]
            y = self.output_heads['MLM'](y)
            y = y.view(-1, self.encoder.config.vocab_size)
            loss = F.cross_entropy(y, X['label'].view(-1))
            return y, loss
        elif self.output_type == 'Supervised':
            y = y["last_hidden_state"]
            y *= X['attn_masks'][:,:,None]
            y = y.sum(1)
            y /= (X['attn_masks'].sum(-1)[:,None].clamp(min=1))
            y = self.output_heads['Supervised'](y)
            loss = F.cross_entropy(y, X['label'])
            return y, loss
        else:
            raise NotImplementedError(f'unknown head type {self.output_type}')