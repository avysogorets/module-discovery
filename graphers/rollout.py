import torch
from .grapher_base import GrapherBase
from ..globals import *


class Rollout(GrapherBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        assert self.collection_level == LAYER
        assert self.aggregation_level in [MODEL, DATASET]

    def aggregate2level(self, attention, **kwargs):
        attention = attention.sum(2)
        B, L, S, _ = attention.shape
        out_attentions = torch.zeros(B, S, S)
        device = attention.device
        for b in range(B):
            attn = attention[b,:,:,:]
            residual_att = torch.eye(S).to(device)
            aug_att_mat = attn + residual_att[None,...]
            aug_att_mat = aug_att_mat / aug_att_mat.sum(dim=-1)[...,None]
            joint_attentions = torch.zeros(aug_att_mat.shape).to(device)
            layers = joint_attentions.shape[0]
            joint_attentions[0] = aug_att_mat[0]
            for i in torch.arange(1, layers):
                joint_attentions[i] = aug_att_mat[i] @ joint_attentions[i-1]
            out_attentions[b,:,:] = joint_attentions[-1]
        return out_attentions
