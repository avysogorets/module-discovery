import torch
import logging
from .grapher_base import GrapherBase
from ..globals import DATASET, MODEL, LAYER, HEAD, ALL


class DecompXBase(GrapherBase):
    """ Warning: this grapher requires much RAM. In its execution, it constructs matrices of
        shape (B x S x S x D). For the smallest GeneFormer, this means 8B GB with float32.
        If include_biases is True, D will mean the expanded dimension of the feed-forward
        net (3072 in BERT, 1024 in GeneFormer). Choose max_batch_size_graph wisely.
    """
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.logger = logging.getLogger(__name__)
        assert self.collection_level >= self.aggregation_level 
        
    def forward(self, model, inp):
        with torch.no_grad():
            _, attention = model(
                X=inp,
                output_attentions=False,
                return_dict=False,
                output_hidden_states=False,
                decompx_config=self.decompx_config)
        attention = torch.stack(attention)
        dim_order = [1, 0] + list(range(2, len(attention.shape)))
        attention = attention.permute(dim_order)
        return attention

    def __call__(self, model, save_path, **kwargs):
        decompx_model = model.decompx
        decompx_model.to(model.dtype)
        decompx_model.to(model.device)
        device_count = torch.cuda.device_count()
        if device_count > 1:
            device_ids = list(range(device_count))
            decompx_model = torch.nn.DataParallel(decompx_model, device_ids=device_ids)
        super().__call__(model=decompx_model, save_path=save_path, **kwargs)

