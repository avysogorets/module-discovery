from .genetics_base import GeneticsDataBase


class Test(GeneticsDataBase):
    """ One sample test
    """
    def __init__(self, max_length, subset_hvg, **model_kwargs):
        super().__init__(max_length=max_length, subset_hvg=subset_hvg)
        self.dataset_name = 'Test'
        self.model_name = model_kwargs['model_name']
        self.model_kwargs = model_kwargs
        self.labels_map = {'NC': 0, 'HD': 1}
        self.num_classes = 2
        self.label_name = 'diagnosis'
        self.gene_identifier = 'ensembl_id'
        self.create_assets()


class Test2(GeneticsDataBase):
    """ Two sample test
    """
    def __init__(self, max_length, subset_hvg, **model_kwargs):
        super().__init__(max_length=max_length, subset_hvg=subset_hvg)
        self.dataset_name = 'Test2'
        self.model_name = model_kwargs['model_name']
        self.model_kwargs = model_kwargs
        self.labels_map = {'NC': 0, 'HD': 1}
        self.num_classes = 2
        self.label_name = 'diagnosis'
        self.gene_identifier = 'ensembl_id'
        self.create_assets()


class Test5(GeneticsDataBase):
    """ Two sample test
    """
    def __init__(self, max_length, subset_hvg, **model_kwargs):
        super().__init__(max_length=max_length, subset_hvg=subset_hvg)
        self.dataset_name = 'Test5'
        self.model_name = model_kwargs['model_name']
        self.model_kwargs = model_kwargs
        self.labels_map = {'NC': 0, 'HD': 1}
        self.num_classes = 2
        self.label_name = 'diagnosis'
        self.gene_identifier = 'ensembl_id'
        self.create_assets()