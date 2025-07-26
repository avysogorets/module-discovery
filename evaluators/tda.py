import numpy as np
import torch
import os
import logging
import json
from multiprocessing import Pool, Process, Queue
from collections import defaultdict
from sklearn import linear_model, preprocessing
from sklearn.metrics import accuracy_score, precision_recall_fscore_support
from .evaluator_base import EvaluatorBase
from ..utils import normalize
from ..external.tda.utils import split_matrices_and_lengths, subprocess_wrap
from ..external.tda.topological import count_top_stats
from ..external.tda.barcode import count_ripser_features, get_only_barcodes, unite_barcodes
from ..external.tda.template import calculate_features_t
from ..globals import MODEL, LAYER, HEAD, TOPOLOGICAL_FEATURES, BARCODE_FEATURES, TEMPLATE_FEATURES


class TDA(EvaluatorBase):
    def __init__(self, device, num_workers, tda_batch_size, save_path, template_ids=None, **kwargs):
        super().__init__(**kwargs)
        assert self.aggregation_level in [MODEL, LAYER, HEAD]
        self.thresholds = [0.025, 0.05, 0.1, 0.25, 0.5, 0.75]
        self.device = device
        self.batch_size = tda_batch_size
        self.template_ids = template_ids
        self.num_workers = num_workers
        self.pool = Pool(num_workers)
        self.stats_cap = 500
        self.logger = logging.getLogger(__name__)
        self.save_path = save_path
        self.Cs = [0.0001, 0.0005, 0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1, 2]
        self.max_iters = [10, 25, 50, 100, 500, 1000, 2000]

    def shapify(self, attentions):
        if self.aggregation_level == MODEL:
            attentions = attentions[:,None,None,:,:]
        if self.aggregation_level == LAYER:
            attentions = attentions[:,:,None,:,:]
        return attentions

    def compute_topological(self, attentions, num_tokens):
        splitted = split_matrices_and_lengths(
            adj_matricies=attentions,
            ntokens=num_tokens,
            num_workers=self.num_workers)
        args = []
        for attentions, num_tokens in splitted:
            args.append((
                attentions,
                self.thresholds,
                num_tokens,
                TOPOLOGICAL_FEATURES,
                self.stats_cap,
                self.aggregation_level))
        # (W) x L x H x F x B' x T 
        stats_tuple_lists_array_part = self.pool.starmap(count_top_stats, args)
        # L x H x F x B x T 
        stats_array = np.concatenate([_ for _ in stats_tuple_lists_array_part], axis=3)
        return stats_array
    
    def compute_barcode(self, attentions, num_tokens):
        queue = Queue()
        barcodes = defaultdict(list)
        splitted = split_matrices_and_lengths(
            adj_matricies=attentions,
            ntokens=num_tokens,
            num_workers=self.num_workers)
        for attentions, num_tokens in splitted:
            p = Process(
                target=subprocess_wrap,
                args=(queue, get_only_barcodes, (attentions, num_tokens, 1, 1e-3)))
            p.start()
            barcodes_part = queue.get()
            p.join()
            p.close()
            barcodes = unite_barcodes(barcodes, barcodes_part)
        num_layers = attentions.shape[1]
        barcodes_array = [[] for _ in range(num_layers)]
        for layer, head in sorted(barcodes.keys()):
            features = count_ripser_features(barcodes[(layer, head)], BARCODE_FEATURES)
            barcodes_array[layer].append(features)
        barcodes_array = np.asarray(barcodes_array)
        return barcodes_array
    
    def compute_template(self, attentions, input_ids):
        features_array = []
        batch_size = attentions.shape[0]
        splitted_indexes = np.array_split(np.arange(batch_size), self.num_workers)
        args = []
        for idxs in splitted_indexes:
            args.append((attentions[idxs], TEMPLATE_FEATURES, input_ids[idxs], self.template_ids))
        features_array = self.pool.starmap(calculate_features_t, args)
        features_array = np.concatenate([_ for _ in features_array], axis=3)
        return features_array
    
    def computeNsave_batch(self, batch_args, split, save_idx):
        attentions = torch.vstack([x for x,_,_ in batch_args])
        input_ids = torch.vstack([x for _,x,_ in batch_args])
        num_tokens = np.concatenate([x for _,_,x in batch_args])
        # Reshape into B x L x H x S x S
        attentions = self.shapify(attentions)
        # compute & save topological features
        stats_array = self.compute_topological(
            attentions=attentions.cpu().numpy(),
            num_tokens=num_tokens)
        filepath = os.path.join(
            self.save_path,
            split,
            'topological',
            f"features_{save_idx}.npy")
        # shape: L x H x F x B x T
        np.save(filepath, stats_array)
        # compute & save ripser (barcode) features
        barcodes_array = self.compute_barcode(
            attentions=attentions.cpu().numpy(),
            num_tokens=num_tokens)
        filepath = os.path.join(
            self.save_path,
            split,
            'barcode',
            f"features_{save_idx}.npy")
        # shape: L x H x B x F
        np.save(filepath, barcodes_array)
        # compute & save template features
        template_array = self.compute_template(
            attentions=attentions.cpu().numpy(),
            input_ids=input_ids.cpu().numpy())
        filepath = os.path.join(
            self.save_path,
            split,
            'template',
            f"features_{save_idx}.npy")
        # shape: L x H x F x B
        np.save(filepath, template_array)

    def prepare_X(self):
        X = {}
        for split in ['train', 'test']:
            X[split] = []
            path = os.path.join(self.save_path, split, 'topological')
            filenames = os.listdir(path)
            load_idx = 0
            filename_part = f'features_{load_idx}.npy'
            while filename_part in filenames:
                topological_part = np.load(
                    os.path.join(self.save_path, split, 'topological', filename_part))
                barcode_part = np.load(
                    os.path.join(self.save_path, split, 'barcode', filename_part))
                template_part = np.load(
                    os.path.join(self.save_path, split, 'template', filename_part))
                num_samples = topological_part.shape[3]
                for i in range(num_samples):
                    features = np.concatenate((
                        topological_part[:,:,:,i,:].flatten(),
                        barcode_part[:,:,i,:].flatten(),
                        template_part[:,:,:,i].flatten()))
                    X[split].append(features)
                load_idx += 1
                filename_part = f'features_{load_idx}.npy'
            # shape: B x D
            X[split] = np.vstack(X[split])
        return X
        
    def train(self, y):
        X = self.prepare_X()
        assert len(X['train']) == len(y['train'])
        assert len(X['test']) == len(y['test'])
        scaler = preprocessing.StandardScaler()
        scaler.fit(X['train'])
        X['train'] = scaler.transform(X['train'])
        X['test'] = scaler.transform(X['test'])
        best_cls = None
        best_acc = 0
        best_parameters = None
        for C in self.Cs:
            for max_iter in self.max_iters:
                classifier = linear_model.LogisticRegression(
                    C=C,
                    penalty='l2',
                    max_iter=max_iter,
                    solver='lbfgs')
                classifier.fit(X['train'], y['train'])
                preds = classifier.predict(X['test'])
                acc = accuracy_score(y['train'], preds)
                if acc > best_acc:
                    best_acc = acc
                    best_cls = classifier
                    best_parameters = (C, max_iter)
        self.logger.info(f'best fit: {best_parameters} with acc.:  {best_acc:.4f}')
        metrics = {}
        for split in ['train', 'test']:
            preds = best_cls.predict(X[split])
            metrics = precision_recall_fscore_support(
                y[split],
                preds,
                average='macro')
            prec, recall, fscore,_ = metrics
            acc = acc = accuracy_score(y[split], preds)
            metrics[split] = {
                'accuracy': acc,
                'recall': recall,
                'precision': prec,
                'fscore': fscore}
        return metrics
    
    def save(self, metrics):
        metrics_filepath = os.path.join(self.save_path, 'results.json')
        f = open(metrics_filepath, 'w')
        json.dump(metrics, f)
        self.logger.info(f'saved asset to {metrics_filepath}')

    def __call__(self, data, graph_path, normalizing_kwargs, **kwargs):
        if not normalizing_kwargs['row_norm']:
            msg = 'template features may be inaccurate with no row normalization'
            self.logger.warning(msg)
        labels = {}
        for split in ['train', 'test']:
            graph_path_split = os.path.join(graph_path, split)
            load_idx = 0
            save_idx = 0
            sample_idx = 0
            filenames = os.listdir(graph_path_split)
            filename_attentions = f'attentions_{load_idx}.pt'
            filename_input_ids = f'input_ids_{load_idx}.pt'
            batch_args = []
            while filename_attentions in filenames:
                filepath = os.path.join(graph_path_split, filename_attentions)
                attentions = torch.load(filepath, map_location=self.device)
                filepath = os.path.join(graph_path_split, filename_input_ids)
                input_ids = torch.load(filepath, map_location=self.device)
                num_tokens = []
                for i in range(attentions.shape[0]):
                    mask = data.datasets[split][sample_idx]['attn_masks']
                    attentions[i] *= mask[None,:]
                    pad_idxs = torch.where(mask == 0)[0]
                    if len(pad_idxs) > 0:
                        num_tokens.append(pad_idxs[0].item())
                    else:
                        num_tokens.append(len(mask))
                    sample_idx += 1
                attentions = normalize(attentions=attentions, **normalizing_kwargs)
                batch_args.append(attentions, input_ids, num_tokens)
                if sum(args[0].shape[0] for args in batch_args) >= self.batch_size:
                    self.computeNsave_batch(batch_args, split, save_idx)
                    batch_args = []
                    save_idx += 1
                load_idx += 1
                filename_attentions = f'attentions_{load_idx}.pt'
                filename_input_ids = f'input_ids_{load_idx}.pt'
            if len(batch_args) > 0:
                self.computeNsave_batch(batch_args, split, save_idx)
            dataset = data.datasets[split]
            labels[split] = [dataset[i]['label'] for i in range(len(dataset))]
        metrics = self.train(labels)
        self.save(metrics)