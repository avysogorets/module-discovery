from .genetics_base import GeneticsDataBase


class IKB(GeneticsDataBase):
    def __init__(self, max_length, subset_hvg, **model_kwargs):
        super().__init__(max_length=max_length, subset_hvg=subset_hvg)
        self.dataset_name = 'IKB'
        self.model_name = model_kwargs['model_name']
        self.model_kwargs = model_kwargs
        self.labels_map = {'ctrl': 0, 'stim': 1}
        self.num_classes = 2
        self.label_name = 'label'
        self.gene_identifier = 'ensembl_id'
        self.create_assets()