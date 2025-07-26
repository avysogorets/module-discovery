class EvaluatorBase:
    def __init__(self, aggregation_level, **kwargs):
        self.modules_required: bool
        self.aggregation_level = aggregation_level

    def save(self, path, **kwargs):
        """ Because each evaluator's output is different,
            it is their responsibility to save assets.
        """
        raise NotImplementedError("implement save method")

    def __call__(self, **kwargs):
        raise NotImplementedError("implement __call__ method")