import numpy as np
import logging
import os
import json
import pickle
from tqdm.auto import tqdm
from sklearn.decomposition import PCA as skPCA
from .evaluator_base import EvaluatorBase
from .utils import train_LR


class PCA(EvaluatorBase):
    def __init__(self, **kwargs):
        self.modules_required = True
        self.logger = logging.getLogger(__name__)

    def save(self, metrics, save_path, id=0):
        metrics_filepath = os.path.join(save_path, f'results_{id}.json')
        with open(metrics_filepath, 'w') as f:
            json.dump(metrics, f)
        self.logger.info(f'saved asset to {metrics_filepath}')

    def prepare_datasets(self, data, partition_idxs):
        X = {'train': [], 'test': []}
        y = {'train': [], 'test': []}
        for split in ['train', 'test']:
            for datum in data.datasets[split]:
                y[split].append(datum['label'])
        for module_idxs in partition_idxs:
            module_matrix = {}
            for split in ['train', 'test']:
                module_matrix[split] = []
                for i in range(len(data.datasets[split])):
                    cell_exprs = data.cell_attrs[split]['exprs'][i]
                    cell_mod_exprs = []
                    for idx in module_idxs:
                        cell_mod_exprs.append(cell_exprs[idx])
                    module_matrix[split].append(cell_mod_exprs)
                module_matrix[split] = np.vstack(module_matrix[split])
                if split == 'train':
                    # hackety hack
                    pca = skPCA(n_components=1)
                    pca.fit(module_matrix[split])
                features = pca.transform(module_matrix[split])[:,0]
                X[split].append(features)
        X['train'] = np.stack(X['train'], axis=1)
        X['test'] = np.stack(X['test'], axis=1)
        return X, y

    def __call__(self, data, modules_pred_path, save_path, **kwargs):
        for filename in tqdm(os.listdir(modules_pred_path), desc='iterating through partitions...'):
            filepath = os.path.join(modules_pred_path, filename)
            if not os.path.isfile(filepath):
                continue
            target_comms = int(filename.rstrip('.pkl').split('_')[1])
            f = open(filepath, 'rb')
            modules = pickle.load(f)
            assert len(modules) == target_comms
            f.close()
            partition_idxs = []
            for module in modules:
                idxs = []
                for idx,gene_id in enumerate(data.gene_ids):
                    if gene_id in module:
                        idxs.append(idx)
                partition_idxs.append(idxs)
            X, y = self.prepare_datasets(data, partition_idxs)
            metrics = train_LR(X, y, logger=self.logger)
            self.save(metrics, save_path, id=target_comms)
                
