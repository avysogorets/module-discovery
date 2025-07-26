import torch
import heapq
import torch.nn.functional as F
from tqdm.auto import tqdm
from collections import Counter
from .moduler_base import ModulerBase
from .utils import create_Ps
from ..globals import HALF, ONE


class DITC(ModulerBase):
    """ Implementation of https://www.jmlr.org/papers/volume3/dhillon03a/dhillon03a.pdf
        - maximizes I(W;Y) if supervised,
        - maximizes I(W;D) otherwise. 
    """
    def __init__(self, device, supervised, **kwargs):
        super().__init__(**kwargs)
        self.supervised = supervised
        self.device = device
        self.max_iters = 100
        self.tolerance = 1e-3
        self.eps = 1e-10

    def __call__(self, data, **kwargs):
        assert not self.supervised or data.supervised
        P_cond, P_marg, good_vocab_ids = create_Ps(data, self.supervised, self.device)
        self.P_cond = P_cond
        self.P_marg = P_marg
        T = self.P_cond.shape[1]
        assert T >= max(self.target_comms_list), f"tokens ({T}) < comms ({max(self.target_comms_list)})"
        self.id2node = {}
        for node in range(len(good_vocab_ids)):
            self.id2node[good_vocab_ids[node]] = node
        id2Cs = []
        for target_comms in tqdm(self.target_comms_list, desc='DITC: iteration over K...'):
            self.node_to_C = torch.argmax(self.P_cond, dim=0)
            Cs = torch.unique(self.node_to_C).tolist()
            self.C2nodes = {C: torch.where(self.node_to_C == C)[0] for C in Cs}
            self.split_C(target_comms=target_comms)
            self.merge_C(target_comms=target_comms)
            old_loss = self.get_loss(target_comms=target_comms)
            delta_loss = 1
            num_iters = 0
            while delta_loss > self.tolerance and num_iters < self.max_iters:
                P_comm = self.get_P_comm(target_comms=target_comms)
                D = self.get_KLD(P_comm=P_comm)
                self.node_to_C = torch.argmin(D, dim=1)
                Cs = torch.unique(self.node_to_C).tolist()
                self.C2nodes = {C: torch.where(self.node_to_C == C)[0] for C in Cs}
                self.split_C(target_comms, split_strategy=ONE)
                new_loss = self.get_loss(target_comms=target_comms)
                delta_loss = old_loss-new_loss
                old_loss = new_loss
                num_iters += 1
            id2C = {}
            for id,node in self.id2node.items():
                id2C[id] = self.node_to_C[node].item()
            id2Cs.append(id2C)
        return id2Cs

    def get_loss(self, target_comms):
        P_comm = self.get_P_comm(target_comms=target_comms)
        D = self.get_KLD(P_comm=P_comm)
        D_weighted = self.P_marg[:,None] * D
        node_to_C_oh = F.one_hot(self.node_to_C, num_classes=target_comms)
        loss = node_to_C_oh * D_weighted
        return loss.sum().item()

    def get_KLD(self, P_comm):
        E = (self.P_cond * torch.log(self.P_cond.clamp(min=self.eps))).sum(dim=0)
        CE = self.P_cond.T @ torch.log(P_comm.clamp(min=self.eps))
        D = E[:,None]-CE
        return D

    def get_P_comm(self, target_comms):
        node_to_C_oh = F.one_hot(self.node_to_C, num_classes=target_comms)
        p_marg_k = (node_to_C_oh.T * self.P_marg[None, :]).sum(dim=1)
        P_comm = self.P_cond @ (node_to_C_oh * self.P_marg[:, None])
        P_comm /= p_marg_k[None, :]
        return P_comm

    def split_C(self, target_comms, split_strategy=HALF):
        self.canonize_C()
        counter = Counter(self.node_to_C.tolist())
        heap = [(-counter[i],i) for i in counter.keys()]
        heapq.heapify(heap)
        C_dst = self.node_to_C.amax().item()+1
        num_iters = max(0, target_comms-len(heap))
        for _ in range(num_iters):
            num_src, C_src = heapq.heappop(heap)
            num_src = -num_src
            if split_strategy == HALF:
                num_dst = num_src // 2
            elif split_strategy == ONE:
                num_dst = 1
            else:
                raise NotImplementedError(f'split strategy {split_strategy} unknown')
            nodes_dst = self.C2nodes[C_src][:num_dst]
            nodes_src = self.C2nodes[C_src][num_dst:]
            self.node_to_C[nodes_dst] = C_dst
            self.C2nodes[C_dst] = nodes_dst
            self.C2nodes[C_src] = nodes_src
            heapq.heappush(heap, (-num_dst, C_dst))
            heapq.heappush(heap, (-(num_src-num_dst), C_src))
            C_dst += 1
        assert len(heap) >= target_comms
        self.canonize_C()

    def merge_C(self, target_comms):
        self.canonize_C()
        counter = Counter(self.node_to_C.tolist())
        heap = [(counter[i],i) for i in counter.keys()]
        heapq.heapify(heap)
        num_iters = max(0, len(heap)-target_comms)
        for _ in range(num_iters):
            num_1, C_1 = heapq.heappop(heap)
            num_2, C_2 = heapq.heappop(heap)
            nodes_1 = self.C2nodes[C_1]
            nodes_2 = self.C2nodes[C_2]
            nodes = torch.cat([nodes_1, nodes_2])
            C_dst = min(C_1,C_2)
            C_src = max(C_1,C_2)
            self.node_to_C[nodes] = C_dst
            self.C2nodes[C_dst] = nodes
            del self.C2nodes[C_src]
            heapq.heappush(heap, (num_1+num_2, C_dst))
        assert len(heap) <= target_comms
        self.canonize_C()

    def canonize_C(self):
        T = len(self.node_to_C)
        Cs = torch.unique(self.node_to_C).tolist()
        C2C = {Co:Cn for Cn,Co in enumerate(Cs)}
        self.node_to_C = self.node_to_C.tolist()
        self.node_to_C = [C2C[self.node_to_C[node]] for node in range(T)]
        self.node_to_C = torch.LongTensor(self.node_to_C)
        self.C2nodes = {C2C[C]: self.C2nodes[C] for C in Cs}
