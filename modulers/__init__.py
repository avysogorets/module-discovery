from ..utils import get_all_subclasses
from .moduler_base import ModulerBase
from .louvain import Louvain
from .spectral import Spectral
from .ib import InformationBottleneck
from .ditc import DITC


def ModulerFactory(moduler_name, **kwargs):
    available_modulers = {}
    for _class_ in get_all_subclasses(ModulerBase):
        available_modulers[_class_.__name__] = _class_
    if moduler_name in available_modulers:
        return available_modulers[moduler_name](**kwargs)
    else:
        raise NotImplementedError(f"undefined moduler {moduler_name}")