from .decompx_base import DecompXBase
from ..external.decompx.decompx_utils import DecompXConfig
from .._scratch.modeling_utils_saved import DecompXConfig as DecompXConfigReference
from ..globals import DATASET, MODEL, LAYER


class DecompX(DecompXBase):
    def __init__(self,
            include_biases,
            mode='absolute',
            aggregation_in_layer='norm',
            aggregation_in_encoder='vector',
            mixing_in_layer=None,
            mixing_in_cls=False,
            **kwargs):
        super().__init__(**kwargs)
        assert self.collection_level in [DATASET, MODEL, LAYER]
        assert not (mixing_in_cls and mixing_in_layer is None)
        if self.collection_level == LAYER:
            output_all_layers = True
        else:
            output_all_layers = False
        self.decompx_config = DecompXConfig(
            collection_level=self.collection_level,
            memory_optimized=True,
            include_biases=include_biases,
            bias_decomp_type='absdot',
            include_res=True,
            include_LN1=True,
            include_FFN=True,
            include_LN2=True,
            norm='L2',
            aggregation_in_layer=aggregation_in_layer,
            aggregation_in_encoder=aggregation_in_encoder,
            mixing_in_layer=mixing_in_layer,
            mixing_in_cls=mixing_in_cls,
            mode=mode,
            output_all_layers=output_all_layers,
            FFN_approx_type="GeLU_ZO",
            include_classifier_w_pooler=False,
            tanh_approx_type="ZO")


class DecompXReference(DecompXBase):
    def __init__(self, include_biases, bias_decomp_type, **kwargs):
        super().__init__(**kwargs)
        assert self.collection_level in [DATASET, MODEL, LAYER]
        self.decompx_config = DecompXConfigReference(
            memory_optimized=False,
            include_biases=include_biases,
            bias_decomp_type=bias_decomp_type,
            include_LN1=True,
            include_FFN=True,
            FFN_approx_type="GeLU_ZO",
            include_LN2=True,
            aggregation='vector',
            include_classifier_w_pooler=False,
            tanh_approx_type="ZO",
            output_all_layers=False,
            output_attention=None,
            output_res1=None,
            output_LN1=None,
            output_FFN=None,
            output_res2=None,
            output_encoder=None,
            output_aggregated='norm',
            output_pooler=None,
            output_classifier=None)