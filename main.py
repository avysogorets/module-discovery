import numpy as np # leave it here
import argparse
import torch
import os
import time
import shutil
import logging
import pickle
import json
from .utils import set_seeds, \
    get_dtype, \
    get_fileid, \
    get_specs_info, \
    id2node_to_modules, \
    normalize
from .graphers.utils import filter_ids
from .config import get_config
from .data import DataFactory
from .models import ModelFactory
from .graphers import GrapherFactory
from .modulers import ModulerFactory
from .evaluators import EvaluatorFactory
from .trainer import Trainer
from .globals import *

parser = argparse.ArgumentParser()

# general parameters
parser.add_argument('--save', default=False, action='store_true', help='save output files')
parser.add_argument('--use_gpu', default=False, action='store_true', help='whether to use GPU (if available)')
parser.add_argument('--seed', type=int, default=42, help='global seed')
parser.add_argument('--res_path', type=str, default=f'{DUMP_DIR}/res', help='path to results/output')
parser.add_argument('--log_path', type=str, default=f'{PROJECT_DIR}/log', help='path to execution logs')

# dataset parameters
parser.add_argument('--dataset_name', type=str, default='IKB', help='name of the dataset')
parser.add_argument('--max_length', type=int, default=MAX_LEN, help='maximum input length')
parser.add_argument('--subset_hvg', type=int, default=False, help='[scGPT] subset a number of highly variable genes')
parser.add_argument('--num_groups', type=int, default=2, help='[Switches] number of groups of tokens (classes)')
parser.add_argument('--num_tokens', type=int, default=8, help='[Switches] number of regular tokens in vocab')
parser.add_argument('--add_cls', default=False, action='store_true', help='[Switches] <cls> or mean pooling')

# model parameters
parser.add_argument('--model_name', type=str, default='GeneFormer', help='name of the model')
parser.add_argument('--geneformer_name', type=str, default='gf-12L-30M-i2048', help='[GeneFormer] pretraind model')
parser.add_argument('--scgpt_name', type=str, default='general', help='[scGPT] general | brain')
parser.add_argument('--backbone_name', type=str, default='bert-base-uncased', help='[BERT]')
parser.add_argument('--num_layers', type=int, default=1, help='[Tiny] number of hidden layers')

# train parameters
parser.add_argument('--trainer_name', type=str, default=None, help='finetuning procedure (if any): None | Supervised | MLM')
parser.add_argument('--epochs', type=int, default=20, help='epochs for training')
parser.add_argument('--lr_encoder', type=float, default=1e-5, help='learning rate for the encoder')
parser.add_argument('--lr_head', type=float, default=0.01, help='learning rate for the output head')
parser.add_argument('--decay_encoder', type=float, default=1e-4, help='L2 weight decay for encoder')
parser.add_argument('--decay_head', type=float, default=1e-4, help='L2 weight decay for the output head')
parser.add_argument('--max_batch_size_train', type=int, default=32, help='maximum batch size for a single backward pass')
parser.add_argument('--batch_size', type=int, default=16, help='batch size for training')
parser.add_argument('--dtype_train', type=str, default='float32', help='weights precision during training')

# grapher parameters
parser.add_argument('--graph_path', type=str, default=None, help='path to directory with existing graphs')
parser.add_argument('--grapher_name', type=str, default=None, help='name of the grapher')
parser.add_argument('--aggregation_level', type=int, default=DATASET, help=f'[Token attributions scope] {DATASET} | {MODEL} | {LAYER} | {HEAD}')
parser.add_argument('--collection_level', type=int, default=DATASET, help=f'[DecompXBase] collection scope: {DATASET} | {MODEL} | {LAYER} | {HEAD}')
parser.add_argument('--process_before_aggregation', default=False, action='store_true', help=f'compute correlations or distances before aggregating')
parser.add_argument('--process_before_accumulation', default=False, action='store_true', help=f'compute correlations or distances before accumulating')
parser.add_argument('--remove_eye', default=False, action='store_true', help=f'remove main diagonal when computing correlations')
parser.add_argument('--remove_cls', default=False, action='store_true', help=f'remove the first token when computing correlations')
parser.add_argument('--metric', type=str, default='L2', help=f'[Embeddings] Distance metric: L2 | cosine')
parser.add_argument('--max_batch_size_graph', type=int, default=1, help='maximum batch size for a single backward pass')
parser.add_argument('--limit_samples_frac', type=float, default=1., help='limit frac. of samples used to compute graph (< 1 for DATASET agg. only)')
parser.add_argument('--dtype_graph', type=str, default='float32', help='dtype of edge values')
parser.add_argument('--layer', type=int, default=ALL, help=f'attention matrix from layer')
parser.add_argument('--include_biases', type=bool, default=False, help='[DecompXBase] include biases into factorization or not')
parser.add_argument('--mode', type=str, default='absolute', help='[DecompXBase] scores computation: absolute | relative')
parser.add_argument('--aggregation_in_layer', type=str, default=None, help='[DecompXBase] scores from vectors: norm | dot | None')
parser.add_argument('--mixing_in_layer', type=str, default=None, help='[DecompXBase] scores from vectors: dot | angle | None')
parser.add_argument('--mixing_in_cls', default=False, action='store_true', help='[DecompXBase] whether to compute angle/dot in CLS embedding only')
parser.add_argument('--aggregation_in_encoder', type=str, default='norm', help='[DecompXBase] scores from layers: None | rollout | vector')
parser.add_argument('--num_bins', type=int, default=None, help='[MI] number of exression quantiles for GeneticsDataBase dataset')

# normalizing parameters
parser.add_argument('--frequency', default=False, action='store_true', help='normalize interactions by counts')
parser.add_argument('--negatives', type=str, default=None, help='how to deal with negatives: None | clamp | addmin')
parser.add_argument('--row_norm', default=False, action='store_true', help='normalize rows to sum = 1')
parser.add_argument('--sinkhorn', default=False, action='store_true', help='normalize rows & cols to sum = 1')
parser.add_argument('--uniform', default=False, action='store_true', help='standardize so that max = 1')
parser.add_argument('--transpose', default=False, action='store_true', help='transpose interaction matrix')
parser.add_argument('--zero_diag', default=False, action='store_true', help='zero out main diagonal before normalizing')
parser.add_argument('--correlation', default=False, action='store_true', help='interactions as correlation')

# moduler parameters
parser.add_argument('--moduler_name', type=str, default=None, help='community detection algorithm name')
parser.add_argument('--target_comms_list', nargs='+', type=int, help='list of numbers of clusters')
parser.add_argument('--supervised', default=False, action='store_true', help='[InformationBottleneck, DITC] maximize I(W;D): False, or I(W;Y): True')
parser.add_argument('--allow_signed', default=False, action='store_true', help='[Louvain] allow signed affinity matrix to be passed to Louvain')

# evaluator parameters
parser.add_argument('--evaluator_name', type=str, default=None, help='name of the evaluator')
parser.add_argument('--eval_dataset_name', type=str, default=None, help='evaluation dataset')
parser.add_argument('--freeze_epochs', type=int, default=10, help='[Freeze] training epochs for FreezeModel')
parser.add_argument('--freeze_baseline', default=False, action='store_true', help='[Freeze] allow attention training')
parser.add_argument('--freeze_factor', type=int, default=1, help='[Freeze] model downsizing factor')
parser.add_argument('--pathways_filepath', type=str, default=f'{PROJECT_DIR}/assets/genetics/pathways.csv')
parser.add_argument('--tda_batch_size', type=int, default=128, help='[TDAKus] data processing batch size')

args = parser.parse_args()


if __name__ == "__main__":

    # general setup
    global_start = time.time()
    num_gpus = torch.cuda.device_count()
    if num_gpus > 0 and args.use_gpu:
        device = torch.device('cuda:0')
    else:
        device = torch.device('cpu')
    print(f'[device: {device} is ready]')
    os.makedirs(args.res_path, exist_ok=True)
    os.makedirs(args.log_path, exist_ok=True)
    set_seeds(args.seed)
    model_kwargs = get_config(args, auto_config=True)
    fileid = get_fileid(args, stage=ALL)
    general_path = os.path.join(args.res_path, get_fileid(args))
    os.makedirs(general_path, exist_ok=True)

    # logger setup
    log_filename = '.'.join([fileid, 'log'])
    log_filepath = os.path.join(args.log_path, log_filename)
    logging.basicConfig(
            filename=log_filepath,
            filemode='w',
            level=logging.INFO)
    print(f'[logs saved to {log_filepath}]')
    logger = logging.getLogger(__name__)
    specs_info = get_specs_info(args)
    logger.info(specs_info)

    # data setup
    data = DataFactory(
        dataset_name=args.dataset_name,
        max_length=args.max_length,
        subset_hvg=args.subset_hvg,
        num_groups=args.num_groups,
        num_tokens=args.num_tokens,
        add_cls=args.add_cls,
        **model_kwargs)
    
    # model setup
    model = ModelFactory(
        model_name=args.model_name,
        num_classes=data.num_classes,
        num_layers=args.num_layers,
        data=data,
        model_kwargs=model_kwargs,
        device=device,
        dtype=get_dtype(args.dtype_train))
    model.set_output_type(args.trainer_name)
    
    # evaluator setup
    if args.evaluator_name is not None:
        evaluator = EvaluatorFactory(
            args.evaluator_name,
            config=model.encoder.config,
            aggregation_level=args.aggregation_level,
            device=device,
            tda_batch_size=args.tda_batch_size,
            num_workers=8)
    else:
        evaluator = None
    
    # training (if applicable)
    logger.info(f'entering training stage...    total time taken: {(time.time()-global_start) / 60:.1f} min.')
    model_path = os.path.join(general_path, get_fileid(args, stage="model"))
    os.makedirs(model_path, exist_ok=True)
    model_filepath = os.path.join(model_path, 'model.pt')
    vocab_filepath = os.path.join(model_path, 'token2i.pkl')
    with open(vocab_filepath, 'wb') as f:
        pickle.dump(data.token_dict, f)
    if os.path.exists(model_filepath):
        logger.info(f'using existing assets at {model_filepath}')
    else:
        if args.trainer_name is not None:
            trainer = Trainer(
                trainer_name=args.trainer_name,
                model=model,
                data=data,
                lr_head=args.lr_head,
                lr_encoder=args.lr_encoder,
                decay_head=args.decay_head,
                decay_encoder=args.decay_encoder,
                epochs=args.epochs,
                batch_size=args.batch_size,
                max_batch_size=args.max_batch_size_train,
                num_workers=min(torch.cpu.device_count(), args.max_batch_size_train // 2),
                verbose=True)
            trainer.train()
            metrics_filepath = os.path.join(model_path, 'metrics.json')
            with open(metrics_filepath, 'w') as f:
                json.dump(trainer.metrics_history, f)
        state_dict = model.state_dict()
        torch.save(state_dict, model_filepath)
    state_dict = torch.load(model_filepath, map_location=device)
    model.load_state_dict(state_dict)
    model.set_output_type(None)

    # graphing
    logger.info(f'entering graphing stage...    total time taken: {(time.time()-global_start) / 60:.1f} min.')
    if args.graph_path is not None:
        graph_path = args.graph_path
        graph_load_path = args.graph_path
    else:
        graph_path = get_fileid(args, stage="graph")
        graph_load_path = os.path.join(model_path, graph_path)
    graph_save_path = os.path.join(model_path, graph_path)
    os.makedirs(graph_save_path, exist_ok=True)
    graph_load_path_split = {
        'train': os.path.join(graph_load_path, 'train'),
        'test': os.path.join(graph_load_path, 'test')}
    if os.path.exists(graph_load_path_split['train']) and len(os.listdir(graph_load_path_split['train'])) > 0:
        logger.info(f"using existing assets at {graph_load_path}")
    elif args.grapher_name is None:
        logger.info(f'skipping graph creation...')
    else:
        model.dtype = get_dtype(args.dtype_graph)
        model.to(get_dtype(args.dtype_graph))
        grapher = GrapherFactory(
            args.grapher_name,
            data=data,
            layer=args.layer,
            aggregation_level=args.aggregation_level,
            collection_level=args.collection_level,
            max_batch_size=args.max_batch_size_graph,
            limit_samples_frac=args.limit_samples_frac,
            include_biases=args.include_biases,
            mode=args.mode,
            aggregation_in_layer=args.aggregation_in_layer,
            aggregation_in_encoder=args.aggregation_in_encoder,
            mixing_in_layer=args.mixing_in_layer,
            mixing_in_cls=args.mixing_in_cls,
            process_before_aggregation=args.process_before_aggregation,
            process_before_accumulation=args.process_before_accumulation,
            negatives=args.negatives,
            remove_eye=args.remove_eye,
            remove_cls=args.remove_cls,
            metric=args.metric,
            num_bins=args.num_bins,
            num_workers=min(torch.cpu.device_count(), args.max_batch_size_graph // 2),
            device=torch.device('cpu'))
        os.makedirs(graph_load_path_split['train'], exist_ok=True)
        grapher(model=model, split='train', save_path=graph_load_path_split['train'])
        if data.has_test:  
            os.makedirs(graph_load_path_split['test'], exist_ok=True)
            grapher(model=model, split='test', save_path=graph_load_path_split['test'])            

    # moduling
    logger.info(f'entering moduling stage...    total time taken: {(time.time()-global_start) / 60:.1f} min.')
    normalize_fileid = get_fileid(args, stage="normalize")
    graph_save_normalized_path = os.path.join(graph_save_path, normalize_fileid)
    os.makedirs(graph_save_normalized_path, exist_ok=True)
    normalizing_kwargs = {
        'frequency': args.frequency,
        'negatives': args.negatives,
        'row_norm': args.row_norm,
        'sinkhorn': args.sinkhorn,
        'uniform': args.uniform,
        'transpose': args.transpose,
        'zero_diag': args.zero_diag,
        'correlation': args.correlation}
    module_fileid = get_fileid(args, stage="module")
    module_path = os.path.join(graph_save_normalized_path, module_fileid)
    os.makedirs(module_path, exist_ok=True)
    if args.moduler_name is None or evaluator is None or not evaluator.modules_required:
        logger.info(f'skipping module creation...')
    else:
        assert args.aggregation_level == DATASET
        if args.graph_path is not None or args.grapher_name is not None:
            attentions = torch.load(
                os.path.join(graph_load_path_split['train'], 'attentions_0.pt'),
                map_location=device)
            assert attentions.is_sparse, f"store DATASET-level assets in a sparse format"
            attentions = attentions.to_dense()
            two_counts = torch.load(
                os.path.join(graph_load_path_split['train'], 'two_counts_0.pt'),
                map_location=device)
            assert two_counts.is_sparse, f"store DATASET-level assets in a sparse format"
            two_counts = two_counts.to_dense()
            vocab_ids_to_local_ids = torch.load(
                os.path.join(graph_load_path_split['train'], 'vocab_ids_to_local_ids_0.pt'),
                map_location=device)
            assert vocab_ids_to_local_ids.is_sparse, f"store DATASET-level assets in a sparse format"
            vocab_ids_to_local_ids = vocab_ids_to_local_ids.to_dense()
            good_local_ids, good_vocab_ids, good_tokens = filter_ids(
                matrix=attentions.cpu().numpy(),
                token_dict=data.token_dict,
                vocab_ids_to_local_ids=vocab_ids_to_local_ids)
            attentions_good = attentions[good_local_ids,:][:,good_local_ids]
            two_counts_good = two_counts[good_local_ids,:][:,good_local_ids]
            attentions_good = normalize(
                attentions=attentions_good,
                two_counts=two_counts_good,
                aggregation_level=args.aggregation_level,
                **normalizing_kwargs)
        else:
            attentions_good = None
            good_vocab_ids = None
        args.target_comms_list = sorted(args.target_comms_list)
        moduler = ModulerFactory(
            args.moduler_name,
            target_comms_list=args.target_comms_list,
            supervised=args.supervised,
            allow_signed=args.allow_signed,
            device=device)
        id2nodes = moduler(adjacency=attentions_good, ids=good_vocab_ids, data=data)
        for i in range(len(args.target_comms_list)):
            modules = id2node_to_modules(id2nodes[i])
            target_comms = args.target_comms_list[i]
            assert len(modules) == target_comms, f"num modules {len(modules)} != target comms {target_comms}"
            module_filepath = os.path.join(module_path, f'modules_{target_comms}.pkl')
            with open(module_filepath, 'wb') as f:
                pickle.dump(modules, f)

    # evaluating
    if evaluator is not None:
        if args.eval_dataset_name != args.dataset_name:
            eval_data = DataFactory(
                dataset_name=args.eval_dataset_name,
                max_length=args.max_length,
                subset_hvg=args.subset_hvg,
                num_groups=args.num_groups,
                num_tokens=args.num_tokens,
                add_cls=args.add_cls,
                **model_kwargs)
        else:
            eval_data = data
        logger.info(f'entering evaluation stage...  total time taken: {(time.time()-global_start) / 60:.1f} min.')
        eval_fileid = get_fileid(args, stage="evaluate")
        if evaluator.modules_required:
            eval_path = os.path.join(module_path, eval_fileid)
        else:
            eval_path = os.path.join(graph_save_normalized_path, eval_fileid)
        os.makedirs(eval_path, exist_ok=True)
        model = ModelFactory(
            model_name=args.model_name,
            num_classes=eval_data.num_classes,
            data=eval_data,
            model_kwargs=model_kwargs,
            device=device,
            dtype=get_dtype(args.dtype_train))
        training_kwargs = {
            'lr_encoder': args.lr_encoder,
            'lr_head': args.lr_head,
            'batch_size': args.batch_size,
            'decay_encoder': args.decay_encoder,
            'decay_head': args.decay_head,
            'epochs': args.freeze_epochs}
        evaluator(
            data=eval_data,
            graph_load_path_split=graph_load_path_split,
            save_path=eval_path,
            freeze_baseline=args.freeze_baseline,
            freeze_factor=args.freeze_factor,
            embedder=model.embedder if hasattr(model, 'embedder') else None,
            modules_pred_path=module_path,
            modules_true_path=args.pathways_filepath,
            world_size=len(data.token_dict),
            device=device,
            normalizing_kwargs=normalizing_kwargs,
            training_kwargs=training_kwargs)
    
    logger.info('A')
    logger.info('V')
    print('\nA\nV')