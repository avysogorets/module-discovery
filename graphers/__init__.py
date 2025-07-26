from .grapher_base import GrapherBase
from .raw import Raw
from .rollout import Rollout
from .nba import NormBased
from .decompx import DecompX
from .globenc import GlobEnc
from .alti import ALTI
from .embeddings import Embeddings
from .twocounts import TwoCounts
from .mi import MI
from ..utils import get_all_subclasses


def GrapherFactory(grapher_name, **kwargs):
    available_graphers = {}
    for _class_ in get_all_subclasses(GrapherBase):
        available_graphers[_class_.__name__] = _class_
    if grapher_name in available_graphers:
        return available_graphers[grapher_name](**kwargs)
    else:
        raise NotImplementedError(f"undefined grapher {grapher_name}")