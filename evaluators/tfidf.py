import logging
import os
import json
from sklearn.feature_extraction.text import TfidfVectorizer
from .evaluator_base import EvaluatorBase
from .utils import train_LR


class TFIDF(EvaluatorBase):
    def __init__(self, **kwargs):
        self.modules_required = True
        self.logger = logging.getLogger(__name__)

    def prepare_datasets(self, data, id2node):
        X = {}
        y = {}
        special_ids = []
        for token in data.special_tokens.values():
            if token is None:
                continue
            special_ids.append(data.token_dict[token])
        unk_node = max(id2node.values())+1
        vectorizer = TfidfVectorizer(stop_words=None)
        for split in ['train', 'test']:
            string_list = []
            label_list = []
            for datum in data.datasets[split]:
                node_list = []
                for id in datum['input_ids']:
                    id = id.item()
                    if id in special_ids:
                        continue
                    if id not in id2node:
                        node = unk_node
                    else:
                        node = id2node[id]
                    # this is a necessary workaround
                    node = 'a' + str(node)
                    node_list.append(node)
                string_list.append(' '.join(node_list))
                label_list.append(datum['label'])
            if split == 'train':
                vectorizer.fit(string_list)
            features = vectorizer.transform(string_list).toarray()
            X[split] = features
            y[split] = label_list
        return X, y
    
    def save(self, metrics, save_path):
        metrics_filepath = os.path.join(save_path, 'results.json')
        with open(metrics_filepath, 'w') as f:
            json.dump(metrics, f)
        self.logger.info(f'saved asset to {metrics_filepath}')

    def __call__(self, data, save_path, id2node, **kwargs):
        X, y = self.prepare_datasets(data, id2node)
        metrics = train_LR(X, y, self.logger)
        self.save(metrics=metrics, save_path=save_path)



                    