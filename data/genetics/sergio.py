from .genetics_base import GeneticsDataBase


class SERGIO(GeneticsDataBase):
    def __init__(self, max_length, subset_hvg, **model_kwargs):
        super().__init__(max_length=max_length, subset_hvg=subset_hvg)
        self.dataset_name = 'SERGIO'
        self.model_name = model_kwargs['model_name']
        self.model_kwargs = model_kwargs
        self.gene_identifier = 'ensembl_id'
        self.create_assets()