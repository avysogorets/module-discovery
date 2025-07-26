import logging
import os
import json
import pickle
from tqdm.auto import tqdm
from sklearn.feature_extraction.text import TfidfVectorizer, CountVectorizer
from .evaluator_base import EvaluatorBase
from .utils import train_LR, train_NB
from ..utils import modules_to_id2node


class Linear(EvaluatorBase):
    def __init__(self, **kwargs):
        self.modules_required = True
        self.logger = logging.getLogger(__name__)

    def prepare_datasets(self, data, id2node):
        X = {'tfidf': {}, 'count': {}}
        y = {}
        special_ids = []
        for token in data.special_tokens.values():
            if token is None:
                continue
            special_ids.append(data.token_dict[token])
        unk_node = max(id2node.values())+1
        vectorizers = {
            'tfidf': TfidfVectorizer(stop_words=None),
            'count': CountVectorizer(stop_words=None)}
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
                    # hackety hack
                    node = 'a' + str(node)
                    node_list.append(node)
                string_list.append(' '.join(node_list))
                label_list.append(datum['label'])
            if split == 'train':
                vectorizers['tfidf'].fit(string_list)
                vectorizers['count'].fit(string_list)
            X['tfidf'][split] = vectorizers['tfidf'].transform(string_list).toarray()
            X['count'][split] = vectorizers['count'].transform(string_list).toarray()
            y[split] = label_list
        return X, y
    
    def save(self, metrics, save_path, id):
        metrics_filepath = os.path.join(save_path, f'results_{id}.json')
        with open(metrics_filepath, 'w') as f:
            json.dump(metrics, f)
        self.logger.info(f'saved asset to {metrics_filepath}')

    def __call__(self, data, save_path, modules_pred_path, **kwargs):
        assert data.supervised, f"Linear evaluator requires labels"
        assert data.datasets['test'] is not None, "Linear evaluator requires test split"
        for filename in tqdm(os.listdir(modules_pred_path), desc='Linear: iterating over partitions...'):
            filepath = os.path.join(modules_pred_path, filename)
            if not os.path.isfile(filepath):
                continue
            target_comms = int(filename.rstrip('.pkl').split('_')[1])
            f = open(filepath, 'rb')
            modules = pickle.load(f)
            assert len(modules) == target_comms
            f.close()
            id2node = modules_to_id2node(modules)
            X, y = self.prepare_datasets(data, id2node)
            metrics = train_LR(X['tfidf'], y, self.logger)
            self.save(metrics=metrics, save_path=save_path, id=f'LR_{target_comms}')
            metrics = train_NB(X['count'], y, self.logger)
            self.save(metrics=metrics, save_path=save_path, id=f'NB_{target_comms}')



                    