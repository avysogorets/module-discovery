import numpy as np
import torch
""" It is absolutely critical that numpy is imported before torch;
    otherwise, the execution hangs when computing the norm of A-A.T
    for matrices >= 500 x 500. It seems that th openMP library
    imported by both numpy and torch as a dependency is different.
"""
import time
import os
import multiprocessing as mp
from typing import List, Dict
from .moduler_base import ModulerBase


# os.environ["OMP_NUM_THREADS"] = "1"
# os.environ["MKL_NUM_THREADS"] = "1"


def worker(adjacency, ids, resolution):
    backend = LouvainBackend(resolution)
    partition = backend(adjacency, ids)
    return partition

class Louvain(ModulerBase):
    def __init__(self, allow_signed, allow_directed=True, disable_cpu_exception=True, **kwargs):
        super().__init__(**kwargs)
        self.resolutions = np.linspace(0.2, 1.6, 16)
        self.disable_cpu_exception = disable_cpu_exception
        self.allow_signed = allow_signed
        self.allow_directed = allow_directed
        self.res_min = 0.001
        self.res_max = 3
        self.max_iters = 20
        self.tolerance = 0.02

    def sweep(self, adjacency, ids) -> List[Dict]:
        ############## DEPRECATED - RETIRED ##############
        """ Performs a multiprocessing-backed sweep over all resolutions
            Expects each backend to output a list of id2node dicts.  
        """
        num_workers = min(len(self.resolutions), mp.cpu_count())
        self.pool = mp.Pool(num_workers)
        tasks = []
        for resolution in self.resolutions:
            tasks.append((adjacency.cpu().numpy(), ids, resolution))
        partitions_list = self.pool.starmap(worker, tasks)
        partitions = []
        for partition_list in partitions_list:
            partitions += partition_list
        self.pool.close()
        return partitions

    def binary_search_to_target(self, adjacency, ids, target_comms) -> Dict:
        """ Performs a binary search on the resolution parameter finetuned
            for target_comms.
            Expects each backend to output a list of id2node dicts.
        """
        delta = float('inf')
        num_iters = 0
        while delta > self.tolerance*target_comms and num_iters < self.max_iters:
            res = (self.res_min + self.res_max) / 2
            backend = LouvainBackend(
                resolution=res,
                allow_directed=self.allow_directed,
                allow_signed=self.allow_signed)
            id2node = backend(adjacency.cpu().numpy(), ids)[-1]
            num_modules = len(set(list(id2node.values())))
            delta = abs(num_modules-target_comms)
            num_iters += 1
            self.logger.info(f'[Louvain iter {num_iters}][res: {res:.3f}][comms: {num_modules}][required: {target_comms}]')
            if num_modules > target_comms:
                self.res_max = res
            else:
                self.res_min = res
        return id2node

    def __call__(self, adjacency, ids, **kwargs):
        assert not adjacency.isnan().any(), "NaNs in adjacency"
        assert not adjacency.isinf().any(), "Infs in adjacency" 
        if not self.disable_cpu_exception:
            if torch.cuda.is_available() or mp.cpu_count() < 64:
                self.logger.error('[run Louvain on a CPU node with many cores!]')
                raise RuntimeError("run Louvain on a CPU node with many cores!")
        start = time.time()
        id2nodes = []
        for target_comms in self.target_comms_list:
            id2node = self.binary_search_to_target(adjacency, ids, target_comms)
            id2nodes.append(id2node)
        assert len(id2nodes) <= len(self.target_comms_list)
        for id2node in id2nodes:
            info_msg = (f'[communities: {len(set(list(id2node.values())))}]')
            self.logger.info(info_msg)
        info_msg = f'[total time: {time.time()-start:.2f}]'
        self.logger.info(info_msg)
        return id2nodes


class LouvainBackend:
    """ Much more efficient reimplementation of NetworkX's Louvain algorithm.
        Their graph data strucure requires > 30 GiB for a complete graph with
        15,000 nodes and float32 edges, totaling > 120 GiB required for Louvain.
        This implemetation works with raw adjacency matrix and has no memory
        overhead. Note that working with numpy is significantly faster here; it
        turns out that access operations are almost 100x times faster with numpy
        compared to pytorch. We support both directed and undirected graphs.
    """
    def __init__(self, resolution, allow_directed, allow_signed, threshold=1e-7):
        self.resolution = resolution
        self.threshold = threshold
        self.allow_signed = allow_signed
        self.allow_directed = allow_directed

    def modularity(self, is_directed, is_signed):
        modularity = 0.
        for _, Cnodes in self.C2nodes.items():
            Cnodes = list(Cnodes)
            C_mat = self.adjacency[Cnodes, :][:, Cnodes]
            if is_directed:
                Lc = C_mat.sum()
                d_o = self.d_o[Cnodes].sum()
                d_i = self.d_i[Cnodes].sum()
                modularity += (Lc / self.m - self.resolution * d_i * d_o / self.m**2)
            else:
                Lc = np.tril(C_mat).sum()
                d = self.d[Cnodes].sum()
                modularity += Lc / self.m - self.resolution * d * d / (4*self.m**2)
            if is_signed:
                C_mat_neg = self.adjacency_neg[Cnodes, :][:, Cnodes]
                Lc_neg = np.tril(C_mat_neg).sum()
                d_neg = self.d_neg[Cnodes].sum()
                modularity -= (Lc_neg / self.m_neg - self.resolution * d_neg * d_neg / (4*self.m_neg**2))
        return modularity

    def move(self, node, next_C, is_directed, is_signed):
        prev_C = self.node2C[node]
        if is_directed:
            self.sigma_C_I[next_C] += self.d_i[node]
            self.sigma_C_O[next_C] += self.d_o[node]
            self.weights2comm[:, prev_C] -= (self.adjacency[:, node] + self.adjacency[node, :])
            self.weights2comm[:, next_C] += (self.adjacency[:, node] + self.adjacency[node, :])
            self.weights2comm[node, prev_C] += 2*(self.adjacency[node, node])
            self.weights2comm[node, next_C] -= 2*(self.adjacency[node, node])
        else:
            self.sigma_C[next_C] += self.d[node]
            self.weights2comm[:, prev_C] -= self.adjacency[node]
            self.weights2comm[:, next_C] += self.adjacency[node]
            self.weights2comm[node, prev_C] += self.adjacency[node, node]
            self.weights2comm[node, next_C] -= self.adjacency[node, node]
        if is_signed:
            self.sigma_C_neg[next_C] += self.d_neg[node]
            self.weights2comm_neg[:, prev_C] -= self.adjacency_neg[node]
            self.weights2comm_neg[:, next_C] += self.adjacency_neg[node]
            self.weights2comm_neg[node, prev_C] += self.adjacency_neg[node, node]
            self.weights2comm_neg[node, next_C] -= self.adjacency_neg[node, node]
        delta_num_active_Cs = 0
        if len(self.C2nodes[prev_C]) == 1:
            delta_num_active_Cs -= 1
        if len(self.C2nodes[next_C]) == 0:
            delta_num_active_Cs += 1
        self.C2nodes[prev_C].remove(node)
        self.C2nodes[next_C].add(node)
        self.node2C[node] = next_C
        return delta_num_active_Cs

    def aggregate(self, is_directed, is_signed):
        C_and_Cnodes = list(self.C2nodes.items())
        for C, C_nodes in C_and_Cnodes:
            if len(C_nodes) == 0:
                del self.C2nodes[C]
        new_num_C = len(self.C2nodes.items())
        assert new_num_C == len(set(list(self.node2C.values())))
        new_adjacency = np.zeros((new_num_C, new_num_C), dtype=self.adjacency.dtype)
        C2node = {C:i for i,C in enumerate(self.C2nodes.keys())}
        for C1, C1_nodes in self.C2nodes.items():
            for C2, C2_nodes in self.C2nodes.items():
                C1_C2_mat = self.adjacency[list(C1_nodes),:][:,list(C2_nodes)]
                if is_directed or C1 != C2:
                    val_C1_C2 = C1_C2_mat.sum()
                else:
                    val_C1_C2 = np.tril(C1_C2_mat).sum()
                new_adjacency[C2node[C1], C2node[C2]] = val_C1_C2
                if is_signed:
                    C1_C2_mat_neg = self.adjacency_neg[list(C1_nodes),:][:,list(C2_nodes)]
                    val_C1_C2_neg = np.tril(C1_C2_mat_neg).sum()
                    new_adjacency[C2node[C1], C2node[C2]] -= val_C1_C2_neg
        return C2node, new_adjacency

    def initialize_iter(self, adjacency, is_directed, is_signed):
        self.adjacency = adjacency.copy()
        self.adjacency[self.adjacency < 0] = 0
        num_nodes = adjacency.shape[0]
        self.C2nodes = {}
        self.node2C = {}
        for node in range(num_nodes):
            self.node2C[node] = node
            self.C2nodes[node] = {node}
        self.weights2comm = self.adjacency.copy()
        if is_directed:
            self.weights2comm += self.adjacency.T
            self.sigma_C_I = self.adjacency.sum(0)
            self.sigma_C_O = self.adjacency.sum(1)
            self.d_i = self.adjacency.sum(0)
            self.d_o = self.adjacency.sum(1)
            self.m = np.sum(self.adjacency)
        else:
            self.sigma_C = self.adjacency.sum(0)
            self.sigma_C += np.diagonal(self.adjacency)
            self.d = self.adjacency.sum(0)
            self.d += np.diagonal(self.adjacency)
            self.m = np.tril(self.adjacency).sum()
        # this is to ignore self-loops (following networkx):
        self.weights2comm *= (1-np.eye(num_nodes, num_nodes))
        if is_signed:
            self.adjacency_neg = -adjacency.copy()
            self.adjacency_neg[self.adjacency_neg < 0] = 0
            self.weights2comm_neg = self.adjacency_neg.copy()
            self.sigma_C_neg = self.adjacency_neg.sum(0)
            self.sigma_C_neg += np.diagonal(self.adjacency_neg)
            self.d_neg = self.adjacency_neg.sum(0)
            self.d_neg += np.diagonal(self.adjacency_neg)
            # this is to ignore self-loops (following networkx):
            self.weights2comm_neg *= (1-np.eye(num_nodes, num_nodes))
            self.m_neg = np.tril(self.adjacency_neg).sum()

    def get_remove_cost(self, node, is_directed, is_signed):
        prev_C = self.node2C[node]
        if is_directed:
            remove_cost = (-self.weights2comm[node, prev_C] / self.m
                + (self.resolution / self.m**2)
                * (self.d_i[node]*self.sigma_C_O[prev_C] + self.d_o[node]*self.sigma_C_I[prev_C]))
        else:
            remove_cost = (-self.weights2comm[node, prev_C] / self.m
                + (self.resolution / (2*self.m**2))
                * (self.sigma_C[prev_C]*self.d[node]))
        if is_signed:
            remove_cost_neg = (-self.weights2comm_neg[node, prev_C] / self.m_neg
                + (self.resolution / (2*self.m_neg**2))
                * (self.sigma_C_neg[prev_C]*self.d_neg[node]))
            # minus sign because modularity = modularity(+) - modularity(-)
            remove_cost -= remove_cost_neg
        return remove_cost

    def get_add_gain(self, node, next_C, is_directed, is_signed):
        if is_directed:
            add_gain = (self.weights2comm[node, next_C] / self.m
                - (self.resolution / self.m**2)
                * (self.d_i[node]*self.sigma_C_O[next_C] + self.d_o[node]*self.sigma_C_I[next_C]))
        else:
            add_gain = (self.weights2comm[node, next_C] / self.m
                - (self.resolution / (2*self.m**2))
                * (self.sigma_C[next_C]*self.d[node]))
        if is_signed:
            add_gain_neg = (self.weights2comm_neg[node, next_C] / self.m_neg
                - (self.resolution / (2*self.m_neg**2))
                * (self.sigma_C_neg[next_C]*self.d_neg[node]))
            # minus sign because modularity = modularity(+) - modularity(-)
            add_gain -= add_gain_neg
        return add_gain
    
    # def record_intermediate_id2node(self, target_comms):
    #     C2C = {}
    #    for i,C in enumerate(set(list(self.node2C.values()))):
    #        if len(self.C2nodes[C]) > 0:
    #            C2C[C] = i
    #    assert len(C2C) == target_comms
    #    local_id2node = {}
    #    for id,node in self.id2node.items():
    #        local_id2node[id] = C2C[self.node2C[node]]
    #    self.id2nodes.append(local_id2node)

    def __call__(self, adjacency, ids):
        self.id2nodes = []
        self.id2node = {id:i for i,id in enumerate(ids)}
        is_directed = (torch.norm(adjacency-adjacency.T, p='fro') / torch.norm(adjacency, p='fro')) > self.threshold
        is_signed = adjacency.min() < 0
        assert not (is_directed and is_signed)
        assert self.allow_directed or not is_directed
        assert self.allow_signed or not is_signed
        self.initialize_iter(adjacency, is_directed=is_directed, is_signed=is_signed)
        to_continue = True
        num_active_Cs = len(ids)
        old_modularity = self.modularity(is_directed=is_directed, is_signed=is_signed)
        while to_continue:
            to_continue = False
            N = self.adjacency.shape[0]
            move_count = 1
            while move_count > 0:
                move_count = 0
                nodes = np.random.choice(range(N), size=N, replace=False)
                for node in nodes:
                    prev_C = self.node2C[node]
                    if is_directed:
                        self.sigma_C_I[prev_C] -= self.d_i[node]
                        self.sigma_C_O[prev_C] -= self.d_o[node]
                    else:
                        self.sigma_C[prev_C] -= self.d[node]
                    if is_signed:
                        self.sigma_C_neg[prev_C] -= self.d_neg[node]
                    remove_cost = self.get_remove_cost(node, is_directed=is_directed, is_signed=is_signed)
                    next_Cs = list(self.C2nodes.keys())
                    best_next_C = None
                    best_gain = 0
                    for next_C in next_Cs:
                        if next_C == prev_C:
                            continue
                        if len(self.C2nodes[next_C]) == 0:
                            continue
                        add_gain = self.get_add_gain(node, next_C, is_directed=is_directed, is_signed=is_signed)
                        if best_gain < add_gain + remove_cost:
                            best_gain = add_gain + remove_cost
                            best_next_C = next_C
                    if best_gain > 0:
                        delta_num_active_Cs = self.move(node, best_next_C, is_directed=is_directed, is_signed=is_signed)
                        num_active_Cs += delta_num_active_Cs
                        move_count += 1
                    else:
                        if is_directed:
                            self.sigma_C_I[prev_C] += self.d_i[node]
                            self.sigma_C_O[prev_C] += self.d_o[node]
                        else:
                            self.sigma_C[prev_C] += self.d[node]
                        if is_signed:
                            self.sigma_C_neg[prev_C] += self.d_neg[node]
            C2node, new_adjacency = self.aggregate(is_directed=is_directed, is_signed=is_signed)
            assert len(C2node) == num_active_Cs
            new_id2node = {}
            for id,node in self.id2node.items():
                new_id2node[id] = C2node[self.node2C[node]]
            self.id2node = new_id2node
            new_modularity = self.modularity(is_directed=is_directed, is_signed=is_signed)
            self.initialize_iter(new_adjacency, is_directed=is_directed, is_signed=is_signed)
            if is_signed and self.m_neg == 0:
                is_signed = False
            if new_adjacency.shape[0] < self.adjacency.shape[0]:
                to_continue = True
            if new_modularity-old_modularity <= self.threshold:
                old_modularity = new_modularity
                to_continue = False
            self.id2nodes.append(self.id2node)
        return self.id2nodes