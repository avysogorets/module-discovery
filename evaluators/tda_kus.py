import numpy as np
import torch
import os
import logging
import json
from tqdm.auto import tqdm
from multiprocessing import Pool
from collections import defaultdict
from .evaluator_base import EvaluatorBase
from .utils import shapify, train_LR
from ..utils import normalize
from ..external.tda_kus.utils import split_matrices_and_lengths
from ..external.tda_kus.topological import count_top_stats
from ..external.tda_kus.template import calculate_features_t
from ..globals import MODEL, LAYER, HEAD, TOPOLOGICAL_FEATURES, BARCODE_FEATURES, TEMPLATE_FEATURES


class TDAKus(EvaluatorBase):
    """ TDA from Kushnareva et al., 2022: https://arxiv.org/pdf/2109.04825
    """
    def __init__(self, device, num_workers, tda_batch_size, template_ids=[], **kwargs):
        from ..external.tda_kus.barcode import count_ripser_features
        from ..external.tda_kus.barcode import get_only_barcodes
        from ..external.tda_kus.barcode import unite_barcodes
        super().__init__(**kwargs)
        assert self.aggregation_level in [MODEL, LAYER, HEAD]
        self.count_ripser_features = count_ripser_features
        self.get_only_barcodes = get_only_barcodes
        self.unite_barcodes = unite_barcodes
        self.modules_required = False
        self.thresholds = [0.025, 0.05, 0.1, 0.25, 0.5, 0.75]
        self.device = device
        self.batch_size = tda_batch_size
        self.template_ids = template_ids
        self.num_workers = 16
        self.stats_cap = 500
        self.logger = logging.getLogger(__name__)

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
                self.stats_cap))
        # (W) x L x H x F x B' x T
        pool = Pool(self.num_workers)
        stats_array = pool.starmap(count_top_stats, args)
        pool.close()
        stats_array = [_ for _ in stats_array if 0 not in _.shape]
        # L x H x F x B x T 
        stats_array = np.concatenate([_ for _ in stats_array], axis=3)
        return stats_array
    
    def compute_barcode(self, attentions, num_tokens):
        barcodes = defaultdict(list)
        splitted = split_matrices_and_lengths(
            adj_matricies=attentions,
            ntokens=num_tokens,
            num_workers=self.num_workers)
        # pool.starmap throws issues with ripser... queues might work
        for attentions, num_tokens in splitted:
            barcode_part = self.get_only_barcodes(attentions, num_tokens, 1, 1e-5)
            barcodes = self.unite_barcodes(barcodes, barcode_part)
        num_layers = attentions.shape[1]
        barcodes_array = [[] for _ in range(num_layers)]
        for layer, head in sorted(barcodes.keys()):
            features = self.count_ripser_features(barcodes[(layer, head)], BARCODE_FEATURES)
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
        pool = Pool(self.num_workers)
        features_array = pool.starmap(calculate_features_t, args)
        pool.close()
        features_array = [_ for _ in features_array if 0 not in _.shape]
        features_array = np.concatenate([_ for _ in features_array], axis=3)
        return features_array
    
    def computeNsave_batch(self, batch_args, split, save_idx, save_path):
        attentions = torch.vstack([x for x,_,_ in batch_args])
        input_ids = torch.vstack([x for _,x,_ in batch_args])
        num_tokens = np.concatenate([x for _,_,x in batch_args])
        # Reshape into B x L x H x S x S
        attentions = shapify(attentions, self.aggregation_level)
        # compute & save topological features
        stats_array = self.compute_topological(
            attentions=attentions.cpu().numpy(),
            num_tokens=num_tokens)
        filepath = os.path.join(
            save_path,
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
            save_path,
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
            save_path,
            split,
            'template',
            f"features_{save_idx}.npy")
        # shape: L x H x F x B
        np.save(filepath, template_array)

    def prepare_X(self, save_path):
        X = {}
        for split in ['train', 'test']:
            X[split] = []
            path = os.path.join(save_path, split, 'topological')
            filenames = os.listdir(path)
            load_idx = 0
            filename_part = f'features_{load_idx}.npy'
            while filename_part in filenames:
                topological_part = np.load(
                    os.path.join(save_path, split, 'topological', filename_part))
                barcode_part = np.load(
                    os.path.join(save_path, split, 'barcode', filename_part))
                template_part = np.load(
                    os.path.join(save_path, split, 'template', filename_part))
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
    
    def save(self, metrics, save_path):
        metrics_filepath = os.path.join(save_path, 'results.json')
        with open(metrics_filepath, 'w') as f:
            json.dump(metrics, f)
        self.logger.info(f'saved asset to {metrics_filepath}')

    def __call__(self, data, graph_load_path_split, normalizing_kwargs, save_path, **kwargs):
        if not normalizing_kwargs['row_norm']:
            msg = 'template features may be inaccurate with no row normalization'
            self.logger.warning(msg)
        labels = {}
        for split in ['train', 'test']:
            for feature_type in ['topological', 'barcode', 'template']:
                dir = os.path.join(save_path, split, feature_type)
                os.makedirs(dir, exist_ok=True)
                if len(os.listdir(dir)) > 0:
                    self.logger.info(f'{len(os.listdir(dir))} files found and removed in {dir}.')
                for filename in os.listdir(dir):
                    filepath = os.path.join(dir, filename)
                    if os.path.isfile(filepath):
                        os.remove(filepath)
            save_idx = 0
            sample_idx = 0
            filenames = os.listdir(graph_load_path_split[split])
            max_idx = max(int(filename.split('_')[-1].rstrip('.pt')) for filename in filenames)
            batch_args = []
            for load_idx in tqdm(range(max_idx+1), desc='computing TDA features...'):
                filename_attentions = f'attentions_{load_idx}.pt'
                filename_input_ids = f'input_ids_{load_idx}.pt'
                filepath = os.path.join(graph_load_path_split[split], filename_attentions)
                attentions = torch.load(filepath, map_location=self.device)
                filepath = os.path.join(graph_load_path_split[split], filename_input_ids)
                input_ids = torch.load(filepath, map_location=self.device)
                num_tokens = []
                for i in range(attentions.shape[0]):
                    mask = data.datasets[split][sample_idx]['attn_masks']
                    mask = mask.to(self.device)
                    attentions[i] *= mask[None,:]
                    pad_idxs = torch.where(mask == 0)[0]
                    if len(pad_idxs) > 0:
                        num_tokens.append(pad_idxs[0].item())
                    else:
                        num_tokens.append(len(mask))
                    assert num_tokens[-1] == len(input_ids[i])
                    sample_idx += 1
                attentions = normalize(
                    attentions=attentions,
                    aggregation_level=self.aggregation_level,
                    **normalizing_kwargs)
                assert attentions.max() <= 1, f"max attention ({attentions.max().item()}) exceeds 1"
                assert attentions.min() >= 0, f"min attention ({attentions.min().item()}) is negative"
                batch_args.append((attentions, input_ids, num_tokens))
                if sum(args[0].shape[0] for args in batch_args) >= self.batch_size:
                    self.computeNsave_batch(batch_args, split, save_idx, save_path)
                    batch_args = []
                    save_idx += 1
                filename_attentions = f'attentions_{load_idx}.pt'
                filename_input_ids = f'input_ids_{load_idx}.pt'
            if len(batch_args) > 0:
                self.computeNsave_batch(batch_args, split, save_idx, save_path)
            dataset = data.datasets[split]
            labels[split] = [dataset[i]['label'] for i in range(len(dataset))]
        X = self.prepare_X(save_path)
        metrics = train_LR(X, labels, self.logger)
        self.save(metrics, save_path)