import numpy as np
import logging
import os
import pickle
from math import comb as binom
from tqdm.auto import tqdm
from .evaluator_base import EvaluatorBase
from ..utils import prepare_pathways
from ..globals import DATASET


def fisher_test(mod_pred, mod_true, world_size):
    t_size = len(mod_true)
    f_size = world_size - t_size
    tp_size = len(set(mod_pred).intersection(set(mod_true)))
    tp_size_max = min(len(mod_pred), len(mod_true))
    p = 0
    for tp_xtrm in range(tp_size, tp_size_max+1):
        fp_xtrm = len(mod_pred) - tp_xtrm
        p += binom(t_size, tp_xtrm) * binom(f_size, fp_xtrm)
    p /= binom(world_size, len(mod_pred))
    return p


class FisherTest(EvaluatorBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.modules_required = True
        self.allowed_levels = [DATASET]
        self.logger = logging.getLogger(__name__)

    def save(self, P, save_path, id=0, **kwargs):
        P_filepath = os.path.join(save_path, f'P_{id}.npy')
        np.save(P_filepath, P)
        self.logger.info(f'saved asset to {P_filepath}')

    def __call__(self,
            data,
            modules_pred_path,
            modules_true_path,
            world_size,
            save_path=None,
            **kwargs):
        partition_true = prepare_pathways(modules_true_path, data.token_dict)
        for filename in tqdm(os.listdir(modules_pred_path), desc='iterating through partitions...'):
            filepath = os.path.join(modules_pred_path, filename)
            if not os.path.isfile(filepath):
                continue
            target_comms = int(filename.rstrip('.pkl').split('_')[1])
            f = open(filepath, 'rb')
            gene_mods_pred = pickle.load(f)
            assert len(gene_mods_pred) == target_comms
            f.close()
            N_pred = len(gene_mods_pred)
            N_true = len(partition_true)
            self.P = np.zeros((N_pred, N_true))
            for i in range(N_pred):
                for j in range(N_true):
                    mod_pred = gene_mods_pred[i]
                    mod_true = partition_true[j]
                    self.P[i][j] = fisher_test(mod_pred, mod_true, world_size)
            if save_path:
                self.save(P=self.P, save_path=save_path, id=target_comms)