from .decompx_base import DecompXBase
from ..external.decompx.decompx_utils import DecompXConfig
from ..globals import LAYER, HEAD


class NormBased(DecompXBase):
    def __init__(self,
            include_biases,
            mode,
            aggregation_in_layer,
            aggregation_in_encoder,
            mixing_in_layer,
            mixing_in_cls,
            **kwargs):
        super().__init__(**kwargs)
        assert include_biases == False
        assert aggregation_in_encoder != 'vector'
        assert not (mixing_in_cls and mixing_in_layer is None)
        if self.collection_level == HEAD:
            output_all_layers = True
            memory_optimized = False
        elif self.collection_level == LAYER:
            output_all_layers = True
            memory_optimized = True
        else:
            output_all_layers = False
            memory_optimized = True
            aggregation_in_encoder = aggregation_in_encoder
        self.decompx_config = DecompXConfig(
            collection_level=self.collection_level,
            memory_optimized=memory_optimized,
            include_biases=False,
            bias_decomp_type='absdot',
            include_res=False,
            include_LN1=False,
            include_FFN=False,
            include_LN2=False,
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