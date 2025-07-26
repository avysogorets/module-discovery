import numpy as np
import torch
from tqdm.auto import tqdm
from .grapher_base import GrapherBase
from .twocounts import TwoCounts
from ..data import GeneticsDataBase
from ..data import LanguageDataBase
from ..globals import DATASET


class MI(GrapherBase):
    """ Computes mutual information between pairs of tokens treated as separate r.v.'s
    """
    def __init__(self, num_bins=None, **kwargs):
        super().__init__(**kwargs)
        self.num_bins = num_bins
        self.two_counter = TwoCounts(**kwargs)
        assert self.aggregation_level == DATASET
        assert self.collection_level == DATASET

    def __call__(self, save_path, split='train', **kwargs):
        if self.data.datasets[split] is None:
            self.logger.warning(f'dataset is None for split {split}')
            return
        if isinstance(self.data, GeneticsDataBase):
            A = get_genetics_A(
                self.data,
                num_bins=self.num_bins,
                split=split,
                vocab_ids_to_local_ids=self.vocab_ids_to_local_ids)
            I = get_mi_directly(A, k=self.num_bins)
        elif isinstance(self.data, LanguageDataBase):
            A = get_language_A(
                self.data,
                split=split,
                vocab_ids_to_local_ids=self.vocab_ids_to_local_ids)
            I = get_mi_directly(A, k=2)
        else:
            raise RuntimeError(f'uncategorized dataset {self.data.__class__.__name__}')
        self.two_counter(save_path=None, split=split)
        two_counts = self.two_counter.two_counts
        self.save(A=I, filepath=save_path, two_counts=two_counts)
    

def get_mi_directly(A: torch.LongTensor, k: int):
    """ Computes mutual information for pairs of columns of A.
        A: N x T (design matrix),
        k: number of discrete outcomes of each feature (A is k-valued).
    """
    N, T = A.shape
    P_i = torch.stack([torch.bincount(A[:, t], minlength=k) for t in range(T)], dim=0).float() / N
    I = torch.zeros((T, T), dtype=torch.float32, device=A.device)
    for x in tqdm(range(k), desc='computing mutual informations...'):
        mask_x = (A == x).float()
        for y in range(k):
            mask_y = (A == y).float()
            Pxy = (mask_x.T @ mask_y) / N
            Px = P_i[:, x].unsqueeze(1)
            Py = P_i[:, y].unsqueeze(0)
            PxPy = Px * Py
            with torch.no_grad():
                valid = (Pxy > 0) & (PxPy > 0)
                log_term = torch.zeros_like(Pxy)
                log_term[valid] = torch.log(Pxy[valid] / PxPy[valid])
            I += Pxy * log_term
    return I


def get_genetics_A(data: GeneticsDataBase, num_bins: int, vocab_ids_to_local_ids: torch.LongTensor, split: str):
    """ Computes design matrix A for a GeneticBase dataset with nonzero
        expressions of each gene digitized into equal sized bins (zero is its own bin).
    """
    assert num_bins >= 2
    T = len(data.token_dict)
    N = len(data.datasets[split])
    A = torch.zeros(N, T).long()
    exprs = np.array(data.cell_attrs[split]['exprs'])
    for i,gene_id in enumerate(data.gene_ids):
        quantiles = np.linspace(0, 1, num_bins)
        zero_mask = exprs[:, i] == 0
        nonzero_vals = exprs[~zero_mask, i]
        digitized = np.empty_like(exprs[:,i], dtype=int)
        if len(nonzero_vals) > 0:
            bin_edges = np.quantile(nonzero_vals, quantiles)
            digitized_nonzero = np.digitize(nonzero_vals, bin_edges, right=False)
            digitized_nonzero = np.clip(digitized_nonzero-1, 0, num_bins-2)
            digitized[~zero_mask] = digitized_nonzero+1
        digitized[zero_mask] = 0
        local_id = vocab_ids_to_local_ids[gene_id]
        A[:,local_id] = torch.from_numpy(digitized)
    return A


def get_language_A(data: LanguageDataBase, vocab_ids_to_local_ids: torch.LongTensor, split: str):
    """ Computes design matrix A for a LanguageBase dataset where
        1 indicates co-occurence and 0 otherwise.
    """
    T = len(data.token_dict)
    N = len(data.datasets[split])
    A = torch.zeros(N, T).long()
    for i in range(N):
        local_ids = vocab_ids_to_local_ids[data.datasets[split][i]['input_ids']] # type: ignore
        A[i,local_ids] = 1
    return A


def get_joints(A: torch.LongTensor, k: int):
    """ Computes joint distributions between pairs of columns of A.
        A: N x T (design matrix),
        k: number of discrete outcomes of each feature (A is k-valued).
        Returns p where p[ti,ki,tj,tk] = p(ti=ki,tj=kj).
    """
    N, T = A.shape
    p = torch.zeros(T, k, T, k, dtype=torch.int32, device=A.device)
    for n in tqdm(range(N), desc='computing joint distributions...'):
        a = A[n]
        ti = torch.arange(T)
        tj = torch.arange(T)
        ai = a.view(T, 1).expand(T, T)
        aj = a.view(1, T).expand(T, T)
        p.index_put_(
            (ti[:, None], ai, tj[None, :], aj),
            torch.ones((T, T), dtype=p.dtype, device=A.device),
            accumulate=True)
    p = p.float()
    norm = p.sum((1,3))
    p /= norm[:,None,:,None]
    return p
