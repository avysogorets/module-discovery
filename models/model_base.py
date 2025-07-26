import torch
import copy
import torch.nn.functional as F
from typing import Union, Dict, Tuple
from ..external.decompx.modeling_bert import BertModel as DecompXBertModel
#from .._scratch.modeling_bert_saved import BertModel as DecompXBertModelReference


class ModelBase(torch.nn.Module):
    """ If using BertEncoder for encoder and not a complete BertModel class 
        make sure that attention_mask is in the right format. These masks are
        are transformed into extended masks (e.g., with -10,000 for masked tokens)
        in the forward method of BertModel and input to BertEncoder and BertLayer
        in this internal format. This is particularly important since we extract
        attention matrices from these models so they must be computed correctly.
    """
    def __init__(self, device: torch.device, dtype, **kwargs) -> None:
        super().__init__()
        self.device = device
        self.dtype = dtype
        self.output_type = None
        self.encoder: torch.nn.Module = None
        self.output_heads = torch.nn.ModuleDict({'MLM': None, 'Supervised': None})

    @property
    def decompx(self):
        decompx_bert = DecompXBertModel(
            self.encoder.config,
            add_pooling_layer=False)     
        # https://github.com/huggingface/transformers/issues/6882
        missing_val = torch.arange(self.encoder.config.max_position_embeddings).expand((1, -1))
        missing_val = missing_val.to(self.device)
        missing_val = missing_val.to(self.dtype)
        state_dict = self.encoder.state_dict()
        state_dict["embeddings.position_ids"] = missing_val
        decompx_bert.load_state_dict(state_dict)
        decompx_model = copy.deepcopy(self)
        decompx_model.encoder = decompx_bert
        return decompx_model
    
    def set_output_type(self, new_output_type):
        self.output_type = new_output_type

    def forward(self, X: Union[Dict, torch.Tensor], **kwargs):
        """ Returns, when self.output_size is:
            - 'None': attentions (B x L x H x S x S) and embeddings (B x S x D)
            - 'MLM': logits (B x S x V) and loss
            - 'Supervised': logits (B x K) and loss
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
            y = y["last_hidden_state"][:,0,:]
            y = self.output_heads['Supervised'](y)
            loss = F.cross_entropy(y, X['label'])
            return y, loss
        else:
            raise NotImplementedError(f'unknown head type {self.output_type}')