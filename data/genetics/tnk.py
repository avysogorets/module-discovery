from .genetics_base import GeneticsDataBase


class TNK(GeneticsDataBase):
    def __init__(self, max_length, subset_hvg, **model_kwargs):
        super().__init__(max_length=max_length, subset_hvg=subset_hvg)
        self.dataset_name = 'TNK'
        self.model_name = model_kwargs['model_name']
        self.model_kwargs = model_kwargs
        self.labels_map = {'Pre': 0, 'On': 1}
        self.num_classes = 2
        self.label_name = 'timepoint'
        self.gene_identifier = 'ensembl_id'
        self.create_assets()