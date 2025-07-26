import torch
from transformers import ViTForImageClassification
from .model_base import ModelBase


class ViT(ModelBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)