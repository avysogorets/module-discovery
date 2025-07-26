from ..utils import get_all_subclasses
from .model_base import ModelBase
from .bert import BERT
from .roberta import RoBERTa
from .tiny import Tiny
from .gpt2 import GPT2


def ModelFactory(model_name, **kwargs):
    available_models = {}
    for _class_ in get_all_subclasses(ModelBase):
        available_models[_class_.__name__] = _class_
    if model_name in available_models:
        return available_models[model_name](**kwargs)
    else:
        raise NotImplementedError(f"undefined model {model_name}")