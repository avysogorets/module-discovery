import torch
import logging
import json
import os
from typing import Tuple
import torch.nn.functional as F
import torch.version
from transformers.models.bert.modeling_bert import BertEmbeddings, BertSelfAttention
from transformers.models.bert.modeling_bert import BertForSequenceClassification, BertConfig
from .evaluator_base import EvaluatorBase
from ..utils import normalize
from ..trainer import Trainer
from ..globals import MODEL, DATASET, SPLIT_TEST, SPLIT_TRAIN


##################################################################
################ DEPRECATED - NO LONGER SUPPORTED ################
##################################################################


class Freeze(EvaluatorBase):
    def __init__(self, config, num_workers, device, **kwargs):
        super().__init__(**kwargs)
        self.modules_required = False
        self.config = config
        self.device = device
        self.num_workers = num_workers
        self.logger = logging.getLogger(__name__)
        assert self.aggregation_level in [MODEL, DATASET]

    def loadNnormalize(self, graph_path_split, normalizing_kwargs):
        if self.aggregation_level == DATASET:
            # Deprecated as of 05/15/25
            filepath = os.path.join(graph_path_split['train'], 'attentions_0.pt')
            A = torch.load(filepath, map_location=self.device)
            filepath = os.path.join(graph_path_split['train'], 'two_counts_0.pt')
            two_counts = torch.load(filepath, map_location=self.device)
            A = normalize(
                attentions=A,
                aggregation_level=self.aggregation_level,
                two_counts=two_counts,
                **normalizing_kwargs)
        if self.aggregation_level == MODEL:
            A = {}
            for split in ['train', 'test']:
                A[split] = []
                load_idx = 0
                filename = f'attentions_{load_idx}.pt'
                filenames = os.listdir(graph_path_split[split])
                while filename in filenames:
                    filepath = os.path.join(graph_path_split[split], filename)
                    A[split].append(torch.load(filepath, map_location=self.device))
                    load_idx += 1
                    filename = f'attentions_{load_idx}.pt'
                A[split] = torch.vstack(A[split])
                A[split] = normalize(
                    attentions=A[split],
                    aggregation_level=self.aggregation_level,
                    **normalizing_kwargs)
        return A

    def save(self, metrics, save_path, **kwargs):
        metrics_filepath = '_'.join([save_path, 'metrics.json'])
        f = open(metrics_filepath, 'w')
        json.dump(metrics, f)
        f.close()
        self.logger.info(f'saved asset to {metrics_filepath}')

    def __call__(self,
            data,
            device,
            graph_load_path_split,
            training_kwargs,
            normalizing_kwargs,
            freeze_baseline,
            freeze_factor,
            world_size,
            save_path=None, 
            embedder=None,
            **kwargs):
        if not freeze_baseline:
            A = self.loadNnormalize(graph_path_split=graph_load_path_split, normalizing_kwargs=normalizing_kwargs)
        else:
            A = None
        self.model = FreezeModel(
            A=A,
            world_size=world_size,
            aggregation_level=self.aggregation_level,
            config=self.config,
            device=device,
            num_classes=data.num_classes,
            factor=freeze_factor,
            embedder=embedder)
        trainer = Trainer(
            trainer_name='Supervised',
            model=self.model,
            data=data,
            num_workers=self.num_workers,
            verbose=True,
            max_batch_size=2**10,
            **training_kwargs)
        trainer.train()
        if save_path:
            self.save(
                metrics=trainer.metrics_history,
                save_path=save_path)
            

class FreezeModel(torch.nn.Module):
    def __init__(self, A, world_size, aggregation_level, config, num_classes, device, factor=1, embedder=None):
        super().__init__()
        if embedder is None:
            embedder = BertEmbeddingsWrapper(BertEmbeddings(config))
        self.encoder = FreezeEncoder(
            A=A,
            world_size=world_size,
            aggregation_level=aggregation_level,
            embedder=embedder,
            config=config,
            factor=factor,
            device=device)
        self.classifier = torch.nn.Linear(config.hidden_size // factor, num_classes)
        self.device = device
        self.dtype = torch.float32
        self.to(self.device)
        self.to(self.dtype)

    def forward(self, X, **kwargs):
        y = self.encoder(X)
        y = self.classifier(y)
        loss = F.cross_entropy(y, X['label'])
        return y, loss


class FreezeEncoder(torch.nn.Module):
    def __init__(self, A, world_size, aggregation_level, embedder, config, factor, device):
        super().__init__()
        self.aggregation_level = aggregation_level
        if A is None:
            self.trainable_attention = True
            if self.aggregation_level == DATASET:
                self.A = torch.nn.Parameter(torch.randn(world_size, world_size))
                torch.nn.init.xavier_normal(self.A)
            if self.aggregation_level == MODEL:
                config.num_attention_heads = 1
                self.A = BertSelfAttention(config=config)
        else:
            self.trainable_attention = False
            self.A = A
        self.Wv = torch.nn.Linear(config.hidden_size, config.hidden_size // factor)
        self.embedder = embedder
        self.linear_0 = torch.nn.Linear(config.hidden_size // factor, config.hidden_size // factor)
        self.layer_norm_1 = torch.nn.LayerNorm(config.hidden_size // factor)
        self.linear_1 = torch.nn.Linear(config.hidden_size // factor, config.intermediate_size // factor)
        self.linear_2 = torch.nn.Linear(config.intermediate_size // factor, config.hidden_size // factor)
        self.layer_norm_2 = torch.nn.LayerNorm(config.hidden_size // factor)
        self.device = device
    
    def forward(self, X):
        embeddings = self.embedder(X)
        B, S = X['input_ids'].shape
        if self.trainable_attention and self.aggregation_level == MODEL:
            extended_attention_mask = -10000*(1-X['attn_masks'][:, None, None, :])
            Y = self.A(
                hidden_states=embeddings,
                attention_mask=extended_attention_mask)[0]
        else:
            if self.aggregation_level == DATASET:
                row_indices = X['input_ids'][:,:,None].expand(-1,-1,S)
                col_indices = X['input_ids'][:,None,:].expand(-1,S,-1)
                self.A.data = torch.clamp(self.A.data, min=0)
                A = self.A[row_indices, col_indices]
            if self.aggregation_level == MODEL:
                assert len(X['split'].unique()) == 1, "both train and test samples in a batch?"
                split_code = X['split'][0].item()
                split = None
                if split_code == SPLIT_TRAIN:
                    split = 'train'
                if split_code == SPLIT_TEST:
                    split = 'test'
                A = self.A[split][X['idx']]
            normalizer = torch.sum(A, dim=-1, keepdim=True)
            normalizer[normalizer == 0] = 1
            A = A / normalizer
            A = A * X['attn_masks'][:,None,:]
            assert check_attention_valid(A)
            A = A.to(self.device)
            Y = A @ self.Wv(embeddings)
        Y = self.layer_norm_1(embeddings+self.linear_0(Y))
        Y = self.layer_norm_2(Y+self.linear_2(F.relu(self.linear_1(Y))))
        return Y[:, 0, :]


class BertEmbeddingsWrapper(torch.nn.Module):
    def __init__(self, bert_embeddings):
        super().__init__()
        self.bert_embeddings = bert_embeddings

    def forward(self, X):
        return self.bert_embeddings(input_ids=X['input_ids'])


def check_attention_valid(A):
    assert not A.isinf().any(), "inf values in A"
    assert not A.isnan().any(), "nan values in A"
    assert A.min() >= 0, "negative values in A"
    assert A.max() <= 1, ">1 values in A"
    return True


class TestBert(torch.nn.Module):
    def __init__(self, embedder, device):
        from ..models.scgpt import BertLikeEmbeddings, scGPT
        super().__init__()
        self.embedder = embedder
        self.encoder = BertForSequenceClassification(BertConfig(
            hidden_size=512,
            intermediate_size=512,
            pad_token_id=60694,
            vocab_size=60697,
            num_hidden_layers=8,
            num_attention_heads=8)).bert
        self.classifier = torch.nn.Linear(512, 2)
        self.encoder.embeddings = BertLikeEmbeddings()
        self.device = device
        self.dtype=torch.float32
        self.to(self.device)

    def forward(self, X, embed=False, **kwargs):
        inputs_embeds = self.embedder(X)
        X = self.encoder(
            inputs_embeds=inputs_embeds,
            attention_mask=X['attn_masks'],
            **kwargs)
        if not embed:
            X = X["last_hidden_state"][:,0,:]
            X = self.classifier(X)
            return X
        if isinstance(X, Tuple):
            return X
        else:
            assert "attentions" in X.keys()
            embeddings = X["last_hidden_state"]
            attentions = X["attentions"]
            return embeddings, attentions