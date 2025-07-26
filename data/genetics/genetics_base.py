import os
from ...data.data_base import DataBase
from ...globals import PROJECT_DIR, SPLIT_TRAIN, SPLIT_TEST
from ..utils.utils import TokenizedDataset


class GeneticsDataBase(DataBase):
    def __init__(self, subset_hvg, **kwargs):
        super().__init__(**kwargs)
        self.subset_hvg = subset_hvg
        self.dataset_name = None
        self.gene_identifier = None
        self.model_kwargs = None
        self.labels_map = None
        self.label_name = None
        self.model_name = None
        self.cell_attrs = {}
        self.gene_ids = []

    def create_assets(self, **kwargs):
        loom_path = f'{PROJECT_DIR}/assets/genetics/looms'
        loom_file = f'{self.dataset_name}.loom'
        loom_filepath = os.path.join(loom_path, loom_file)
        if self.model_name == 'GeneFormer':
            from ..utils.geneformer import get_dataset4geneformer
            assert loom_file in os.listdir(loom_path)
            data_list, token_dict, special_tokens, cell_attrs, gene_ids, collate_fns = get_dataset4geneformer(
                loom_filepath=loom_filepath,
                labels_map=self.labels_map,
                label_name=self.label_name,
                gene_identifier=self.gene_identifier,
                model_kwargs=self.model_kwargs,
                max_length=self.max_length)
            self.collate_fns = collate_fns
        elif self.model_name == 'scGPT':
            import anndata
            from ..utils.scgpt import get_dataset4scgpt
            from ..utils.scgpt import loom2anndata
            ann_path = f'{PROJECT_DIR}/assets/genetics/anns'
            ann_file = f'{self.dataset_name}.h5ad'
            ann_filepath = os.path.join(ann_path, ann_file)
            if ann_file not in os.listdir(ann_path):
                assert loom_file in os.listdir(loom_path)
                loom2anndata(loom_filepath)
            adata = anndata.read(ann_filepath)
            data_list, token_dict, special_tokens, cell_attrs, gene_ids, collate_fns = get_dataset4scgpt(
                adata=adata,
                labels_map=self.labels_map,
                label_name=self.label_name,
                gene_identifier=self.gene_identifier,
                model_path=self.model_kwargs['model_path'],
                max_length=self.max_length,
                subset_hvg=self.subset_hvg)
            self.collate_fns = collate_fns
        else:
            raise RuntimeError(f'{self.model_name} not compatible with {self.dataset_name}')
        self.special_tokens = special_tokens
        self.cell_attrs = cell_attrs
        self.gene_ids = gene_ids
        train_idxs = self.get_train_idxs(len(data_list))
        test_idxs = [i for i in range(len(data_list)) if i not in train_idxs]
        train_data_list = [data_list[i] for i in train_idxs]
        test_data_list = [data_list[i] for i in test_idxs]
        self.cell_attrs = {
            'train': {key: [self.cell_attrs[key][i] for i in train_idxs] for key in self.cell_attrs.keys()},
            'test': {key: [self.cell_attrs[key][i] for i in test_idxs] for key in self.cell_attrs.keys()}}
        self.datasets = {
            'train': TokenizedDataset(train_data_list, split_code=SPLIT_TRAIN, global_idxs=train_idxs),
            'test': TokenizedDataset(test_data_list, split_code=SPLIT_TEST, global_idxs=test_idxs)}
        self.token_dict = token_dict