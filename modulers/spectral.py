import torch
from sklearn.cluster import SpectralClustering
from signet.cluster import Cluster
from scipy.sparse import csc_matrix
from .moduler_base import ModulerBase


class Spectral(ModulerBase):
    def __init__(self, allow_signed, allow_directed=True, **kwargs):
        super().__init__(**kwargs)
        self.allow_directed = allow_directed
        self.allow_signed = allow_signed
        self.symmetrizer = 'avg'

    def symmetrize(self, A):
        if self.symmetrizer == 'max':
            return torch.max(A, A.T)
        elif self.symmetrizer == 'avg':
            return (A + A.T) / 2
        elif self.symmetrizer == 'min':
            return torch.min(A, A.T)
        else:
            raise NotImplementedError(f"unknown symmetrizer {self.symmetrizer}")
        
    def __call__(self, adjacency, ids, **kwargs):
        assert not adjacency.isnan().any(), "NaNs in adjacency"
        assert not adjacency.isinf().any(), "Infs in adjacency"
        is_signed = adjacency.amin() < 0
        is_directed = (torch.norm(adjacency-adjacency.T, p='fro') / torch.norm(adjacency, p='fro')) > 1e-5
        assert not (is_directed and is_signed)
        assert self.allow_signed or not is_signed
        if is_directed:
            adjacency = self.symmetrize(adjacency)
            self.logger.info('affinities are directed')
        if is_signed:
            self.logger.info('affinities are signed')
        assert (adjacency-adjacency.T).norm() < 1e-1
        id2Cs = []
        for target_comms in self.target_comms_list:
            if not is_signed:
                moduler = SpectralClustering(
                    n_clusters=target_comms,
                    affinity='precomputed',
                    assign_labels='cluster_qr')
                clustering = moduler.fit(adjacency.cpu().numpy()).labels_
            else:
                Ap = adjacency.clone()
                An = -adjacency.clone()
                Ap[Ap < 0] = 0.
                An[An < 0] = 0.
                Ap = csc_matrix(Ap.cpu().numpy())
                An = csc_matrix(An.cpu().numpy())
                moduler = Cluster(data=(Ap, An))
                clustering = moduler.spectral_cluster_adjacency(k=target_comms)
            id2C = {}
            for i,node in enumerate(clustering):
                id2C[ids[i]] = node
            id2Cs.append(id2C)
            self.log_info(id2C)
        return id2Cs