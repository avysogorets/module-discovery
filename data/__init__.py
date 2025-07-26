from ..utils import get_all_subclasses
from .data_base import DataBase
from .genetics import *
from .language import *
from .synthetic import *


def DataFactory(dataset_name, **kwargs):
    available_datasets = {}
    for _class_ in get_all_subclasses(DataBase):
        available_datasets[_class_.__name__] = _class_
    if dataset_name in available_datasets:
        return available_datasets[dataset_name](**kwargs)
    else:
        raise NotImplementedError(f"undefined dataset <{dataset_name}>")