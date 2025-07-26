import logging
import numpy as np
from typing import Dict, List


class ModulerBase:
    def __init__(self, target_comms_list, **kwargs):
        self.id2node = {}
        self.logger = logging.getLogger(__name__)
        self.target_comms_list = sorted(target_comms_list)

    def log_info(self, id2node):
        node2size = {node: 0 for node in id2node.values()}
        for _,node in id2node.items():
            node2size[node] += 1
        num_modules = len(node2size)
        min_size = min(list(node2size.values()))
        max_size = max(list(node2size.values()))
        avg_size = np.mean(list(node2size.values())).item()
        std_size = np.std(list(node2size.values())).item()
        info = (
            f'[modules {num_modules}][sizes]'
            f'[min {min_size}]'
            f'[max {max_size}]'
            f'[avg {avg_size:.2f}]'
            f'[std {std_size:.2f}]')
        self.logger.info(info)

    def __call__(self, adjacency, ids, **kwargs) -> List[Dict]:
        """ Returns a list of id2node assignments (increasing in #comms)
        """
        raise NotImplementedError("implement Moduler.__call__")