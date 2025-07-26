from .evaluator_base import EvaluatorBase
# from .freeze import Freeze
from .fisher import FisherTest
from .tda_kus import TDAKus
from .linear import Linear
from .pca import PCA
from ..utils import get_all_subclasses


def EvaluatorFactory(evaluator_name, **kwargs):
    available_evaluators = {}
    for _class_ in get_all_subclasses(EvaluatorBase):
        available_evaluators[_class_.__name__] = _class_
    if evaluator_name in available_evaluators:
        return available_evaluators[evaluator_name](**kwargs)
    else:
        raise NotImplementedError(f"undefined evaluator {evaluator_name}")