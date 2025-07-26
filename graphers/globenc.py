from .decompx_base import DecompXBase
from ..external.decompx.decompx_utils import DecompXConfig
from ..globals import DATASET, MODEL, LAYER


class GlobEnc(DecompXBase):
    """ ### indicates changes in config compared to DecompX
    """
    def __init__(self,
            include_biases,
            mode='absolute',
            aggregation_in_layer='norm',
            aggregation_in_encoder='rollout',
            mixing_in_layer=None,
            mixing_in_cls=False,
            **kwargs):
        super().__init__(**kwargs)
        assert self.collection_level in [DATASET, MODEL, LAYER]
        assert not mixing_in_cls
        assert not include_biases
        assert aggregation_in_layer == 'norm'
        assert aggregation_in_encoder == 'rollout'
        assert mixing_in_layer is None
        if self.collection_level == LAYER:
            output_all_layers = True
        else:
            output_all_layers = False
        self.decompx_config = DecompXConfig(
            collection_level=self.collection_level,
            memory_optimized=True,
            include_biases=False,
            bias_decomp_type='absdot',
            include_res=True,
            include_LN1=True,
            include_FFN=False,
            include_LN2=True,
            norm='L2',
            aggregation_in_layer='norm',
            aggregation_in_encoder='rollout',
            mixing_in_layer=None,
            mixing_in_cls=False,
            mode=mode,
            output_all_layers=output_all_layers,
            FFN_approx_type="GeLU_ZO",
            include_classifier_w_pooler=False,
            tanh_approx_type="ZO")