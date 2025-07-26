import torch
from .grapher_base import GrapherBase
from ..globals import *


class Raw(GrapherBase):
    def __init__(self, layer=ALL, **kwargs):
        super().__init__(**kwargs)
        assert self.collection_level == HEAD
        self.layer = layer

    def __call__(self, model, **kwargs):
        device_count = torch.cuda.device_count()
        if device_count > 1:
            device_ids = list(range(device_count))
            dp_model = torch.nn.DataParallel(model, device_ids=device_ids)
            super().__call__(model=dp_model, **kwargs)
        else:
            super().__call__(model=model, **kwargs)
