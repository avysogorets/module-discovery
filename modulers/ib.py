import torch
from tqdm.auto import tqdm
from .moduler_base import ModulerBase
from .utils import create_Ps


class InformationBottleneck(ModulerBase):
    """ Implementation of https://dl.acm.org/doi/10.1145/345508.345578
        - maximizes I(W;Y) if supervised,
        - maximizes I(W;D) otherwise. 
    """
    def __init__(self, device, supervised, **kwargs):
        super().__init__(**kwargs)
        self.supervised = supervised
        self.device = device
        self.eps = 1e-10

    def __call__(self, data, **kwargs):
        assert not self.supervised or data.supervised
        self.create_assets(data)
        T = self.P_cond.shape[1]
        min_target_comms = min(self.target_comms_list)
        assert T >= max(self.target_comms_list), f"tokens ({T}) < comms ({max(self.target_comms_list)})"
        id2Cs = []
        for _ in tqdm(range(T-min_target_comms), desc='agglomerative token merging...'):
            min_val = torch.min(self.deltaI)
            i,j = (self.deltaI == min_val).nonzero()[0]
            i = i.item()
            j = j.item()
            assert i != j
            self.merge(i, j)
            if len(self.C2nodes) not in self.target_comms_list:
                continue
            id2C = {}
            for id,node in self.id2node.items():
                id2C[id] = self.node2C[node]
            id2Cs.append(id2C)
        assert len(self.C2nodes) == min_target_comms
        return id2Cs[::-1]

    def create_assets(self, data):
        P_cond, P_marg, good_vocab_ids = create_Ps(data, self.supervised, self.device)
        self.P_cond = P_cond
        self.P_marg = P_marg
        T = self.P_cond.shape[1]
        for node in range(len(good_vocab_ids)):
            self.id2node[good_vocab_ids[node]] = node
        self.C2nodes = {node: [node] for node in range(T)}
        self.node2C = {node: node for node in range(T)}
        batch_size = 64
        self.deltaI = torch.zeros(T, T)
        for si in tqdm(range(0, T, batch_size), desc='computing initial deltaI...'):
            ei = min(T, si+batch_size)
            P_marg_i = self.P_marg[si:ei]
            P_cond_i = self.P_cond[:,si:ei]
            for sj in range(0, T, batch_size):
                ej = min(T, sj+batch_size)
                P_marg_j = self.P_marg[sj:ej]
                P_cond_j = self.P_cond[:,sj:ej]
                Z = P_marg_i[:,None] + P_marg_j[None,:]
                pi_i = P_marg_i[:,None] / Z
                pi_j = P_marg_j[None,:] / Z
                P_cond_dist_i = P_cond_i[:,:,None]
                P_cond_dist_j = P_cond_j[:,None,:]
                P_bar = pi_i[None,:,:]*P_cond_dist_i + pi_j[None,:,:]*P_cond_dist_j # B x T x T
                KL_i = (P_cond_dist_i*(P_cond_dist_i.clamp(min=self.eps).log()-P_bar.clamp(min=self.eps).log())).sum(dim=0)
                KL_j = (P_cond_dist_j*(P_cond_dist_j.clamp(min=self.eps).log()-P_bar.clamp(min=self.eps).log())).sum(dim=0)
                deltaI_batch = Z*(pi_i*KL_i + pi_j*KL_j)
                self.deltaI[si:ei, sj:ej] = deltaI_batch
        self.deltaI.fill_diagonal_(float('inf'))

    def merge(self, C_1, C_2):
        T = self.P_cond.shape[1]
        C_dst = min(C_1, C_2)
        C_src = max(C_1, C_2)
        P_marg_dst_new = self.P_marg[C_dst] + self.P_marg[C_src]
        P_cond_dst_new = (self.P_marg[C_dst]/P_marg_dst_new) * self.P_cond[:,C_dst]
        P_cond_dst_new += (self.P_marg[C_src]/P_marg_dst_new) * self.P_cond[:,C_src]
        idxs_to_keep = list(range(C_src)) + list(range(C_src+1, T))
        self.P_cond[:,C_dst] = P_cond_dst_new
        self.P_cond = self.P_cond[:,idxs_to_keep]
        self.P_marg[C_dst] = P_marg_dst_new
        self.P_marg = self.P_marg[idxs_to_keep]
        self.deltaI = self.deltaI[idxs_to_keep,:][:,idxs_to_keep]
        Z = self.P_marg[C_dst] + self.P_marg
        pi_i = self.P_marg[C_dst] / Z
        pi_j = self.P_marg / Z
        P_cond_dist_i = self.P_cond[:,[C_dst]*(T-1)]
        P_cond_dist_j = self.P_cond
        P_bar = pi_i[None,:]*P_cond_dist_i + pi_j[None,:]*P_cond_dist_j # N x T
        KL_i = (P_cond_dist_i*(P_cond_dist_i.add(self.eps).log()-P_bar.add(self.eps).log())).sum(dim=0)
        KL_j = (P_cond_dist_j*(P_cond_dist_j.add(self.eps).log()-P_bar.add(self.eps).log())).sum(dim=0)
        deltaI_dst_new = Z*(pi_i*KL_i + pi_j*KL_j)
        self.deltaI[C_dst, :] = deltaI_dst_new
        self.deltaI[:, C_dst] = deltaI_dst_new
        self.deltaI[C_dst, C_dst] = float('inf')
        self.C2nodes[C_dst] += self.C2nodes[C_src]
        for node in self.C2nodes[C_src]:
            self.node2C[node] = C_dst
        self.C2nodes[C_src] = []
        for C in range(C_src+1, T):
            self.C2nodes[C-1] += self.C2nodes[C]
            for node in self.C2nodes[C]:
                self.node2C[node] = C-1
            self.C2nodes[C] = []
        del self.C2nodes[T-1]
        
