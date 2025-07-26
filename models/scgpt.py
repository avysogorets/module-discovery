import torch
import os
import json
import logging
import copy
from typing import Tuple
import torch.nn.functional as F
from transformers.models.bert.modeling_bert import BertConfig, \
    BertForTokenClassification
from scgpt.model import GeneEncoder, \
    ContinuousValueEncoder, \
    ExprDecoder
from scgpt.model.model import CategoryValueEncoder
from scgpt.tokenizer import GeneVocab
from .model_base import ModelBase
from .utils import torch2trans, prepend
from ..globals import *
from ..external.decompx.modeling_bert import BertModel as DecompXBertModel


special_tokens = {
    PAD: '<pad>',
    EOS: '<eoc>',
    CLS: '<cls>'}


class scGPT(ModelBase):
    def __init__(self, model_kwargs, num_classes, **kwargs):
        super().__init__(**kwargs)
        model_path = model_kwargs['model_path']
        model_config_file = open(os.path.join(model_path, 'args.json'), 'r')
        model_kwargs = json.load(model_config_file)
        model_config_file.close()
        vocab = GeneVocab.from_file(os.path.join(model_path, 'vocab.json'))
        for s in special_tokens.values():
            if s not in vocab:
                vocab.append_token(s)
        logger = logging.getLogger(__name__)
        embedder_genes = GeneEncoder(
            num_embeddings=len(vocab),
            embedding_dim=model_kwargs['embsize'],
            padding_idx=vocab[special_tokens[PAD]])
        if model_kwargs['input_emb_style'] == "continuous":
            embedder_values = ContinuousValueEncoder(
                model_kwargs['embsize'],
                model_kwargs['dropout'])
        elif model_kwargs['input_emb_style'] == "category":
            embedder_values = CategoryValueEncoder(
                n_input_bins=model_kwargs['n_bins']+2,
                d_model=model_kwargs['embsize'],
                padding_idx=vocab[special_tokens[PAD]])
        else:
            embedder_values = torch.nn.Identity()
        self.encoder = BertForTokenClassification(BertConfig(
            hidden_size=model_kwargs['embsize'],
            num_attention_heads=model_kwargs['nheads'],
            num_hidden_layers=model_kwargs['nlayers'],
            hidden_act='relu',
            attn_implementation="eager",
            hidden_dropout_prob=0.,
            attention_probs_dropout_prob=0.,
            layer_norm_eps=1e-5,
            intermediate_size=model_kwargs['d_hid'])).bert
        self.encoder.embeddings = BertLikeEmbeddings()
        torch_dict_filepath = os.path.join(model_path, "best_model.pt")
        torch_dict = torch.load(torch_dict_filepath, map_location=self.device)
        embedder_genes_dict = {}
        embedder_values_dict = {}
        encoder_dict = {}
        for param_name in torch_dict.keys():
            module_name = param_name.split('.')[0]
            key_name = '.'.join(param_name.split('.')[1:])
            if module_name == 'encoder':
                embedder_genes_dict[key_name] = torch_dict[param_name]
            elif module_name == 'value_encoder':
                embedder_values_dict[key_name] = torch_dict[param_name]
            elif module_name == 'transformer_encoder':
                encoder_dict[key_name] = torch_dict[param_name]
            else:
                logger.warning(f"unused module {module_name}")
        embedder_genes.load_state_dict(embedder_genes_dict)
        embedder_values.load_state_dict(embedder_values_dict)
        self.embedder = scGPTEmbeddings(embedder_genes, embedder_values)
        self.encoder.load_state_dict(prepend(torch2trans(encoder_dict), 'encoder.'))
        self.output_heads['MLM'] = ExprDecoder(d_model=model_kwargs['embsize'])
        if num_classes:
            self.output_heads['Supervised'] = torch.nn.Linear(model_kwargs['embsize'], num_classes)
        self.to(self.device)
        self.to(self.dtype)

    @property
    def decompx(self):
        decompx_bert = DecompXBertModel(self.encoder.config, add_pooling_layer=False)
        decompx_bert.embeddings = BertLikeEmbeddings()
        decompx_bert.load_state_dict(self.encoder.state_dict())
        decompx_model = copy.deepcopy(self)
        decompx_model.encoder = decompx_bert
        return decompx_model
    
    def forward(self, X, **kwargs):
        inputs_embeds = self.embedder(X)
        y = self.encoder(
            inputs_embeds=inputs_embeds,
            attention_mask=X['attn_masks'],
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
            y = y['pred']
            loss = masked_mse_loss(y, X['label'])
            return y, loss
        elif self.output_type == 'Supervised':
            y = y["last_hidden_state"][:,0,:]
            y = self.output_heads['Supervised'](y)
            loss = F.cross_entropy(y, X['label'])
            return y, loss
        else:
            raise NotImplementedError(f'unknown output type {self.output_type}')
        

class scGPTEmbeddings(torch.nn.Module):
    def __init__(self, embedder_genes, embedder_values):
        super().__init__()
        self.embedder_genes = embedder_genes
        self.embedder_values = embedder_values

    def forward(self, X):
        genes_embs = self.embedder_genes(X['input_ids'])
        values_embs = self.embedder_values(X['values'])
        embs = genes_embs + values_embs
        return embs


class BertLikeEmbeddings(torch.nn.Module):
    def __init__(self):
        super().__init__()

    def forward(self, inputs_embeds, **kwargs):
        return inputs_embeds
    

def masked_mse_loss(input, target):
    idxs_masked = target == -100
    input = input[~idxs_masked]
    target = target[~idxs_masked]
    loss = F.mse_loss(input, target, reduction='mean')
    return loss