import os
import torch
from .globals import *


def get_config(args, auto_config=True):

    assert args.grapher_name is None or args.graph_path is None 
    if args.model_name == 'GeneFormer':
        if auto_config:
            if args.grapher_name in DECOMPX_BASED_GRAPHERS:
                args.max_batch_size_graph = 1
            else:
                args.max_batch_size_graph = 8
            args.max_batch_size_train = 8
            args.lr_encoder = 1e-4
            args.decay_encoder = 1e-4
            args.decay_head = 1e-4
            args.batch_size = 16
            if args.trainer_name == 'MLM':
                args.lr_head = 1e-4
                args.epochs = 3
            if args.trainer_name == 'Supervised':
                args.lr_head = 0.01
                args.epochs = 3
        gf_dir=  f'{UTILS_DIR}/GeneFormer'
        model_size = args.geneformer_name.split('-')[-2]
        gene_median_filepath = f'{gf_dir}/geneformer/gene_dictionaries_{model_size}/gene_median_dictionary_gc{model_size}.pkl'
        token_dict_filepath = f'{gf_dir}/geneformer/gene_dictionaries_{model_size}/token_dictionary_gc{model_size}.pkl'
        model_input_size = int(args.geneformer_name[-4:])
        if model_input_size == 2048:
            assert not args.mixing_in_cls, "GeneFormer with input size 2048 has no CLS"
        model_kwargs = {
            'model_name': args.model_name,
            'model_size': model_size,
            'token_dict_filepath': token_dict_filepath,
            'gene_median_filepath': gene_median_filepath,
            'max_seq_len': model_input_size,
            'encoder_config': {
                'pretrained_model_name_or_path': f'{gf_dir}/{args.geneformer_name}',
                'attn_implementation': 'eager'}}

    if args.model_name == 'scGPT':
        if auto_config:
            if args.grapher_name in DECOMPX_BASED_GRAPHERS:
                args.max_batch_size_graph = 1
            else:
                args.max_batch_size_graph = 8
            args.max_batch_size_train = 32
            args.lr_encoder = 1e-4
            args.decay_encoder = 1e-4
            args.decay_head = 1e-4
            args.batch_size = 32
            if args.trainer_name == 'MLM':
                args.lr_head = 1e-4
                args.epochs = 3
            if args.trainer_name == 'Supervised':
                args.lr_head = 0.01
                args.epochs = 3
        scgpt_dir = f'{UTILS_DIR}/scGPT'
        model_kwargs = {
            'model_name': args.model_name,
            'model_path': os.path.join(scgpt_dir, 'models', args.scgpt_name)}

    if args.model_name == 'BERT':
        args.backbone_name = 'bert-base-uncased'
        if auto_config:
            if args.grapher_name in DECOMPX_BASED_GRAPHERS:
                if args.use_gpu:
                    args.max_batch_size_graph = 2 * torch.cuda.device_count()
                else:
                    args.max_batch_size_graph = 2
            else:
                args.max_batch_size_graph = 32
            if args.grapher_name == 'Embeddings':
                args.max_batch_size_graph = 4
            args.max_batch_size_train = 64
            args.lr_encoder = 5e-5
            args.decay_encoder = 1e-4
            args.decay_head = 1e-4
            args.batch_size = 32
            if args.trainer_name == 'MLM':
                args.lr_head = 1e-4
                args.epochs = 5
            if args.trainer_name == 'Supervised':
                args.lr_head = 0.001
                args.epochs = 3
        backbone_path = f'{PROJECT_DIR}/assets/language/transformers/{args.backbone_name}'
        model_kwargs = {'backbone_path': backbone_path}

    if args.model_name == 'GPT2':
        args.backbone_name = 'gpt2'
        if auto_config:
            if args.grapher_name in DECOMPX_BASED_GRAPHERS:
                args.max_batch_size_graph = 2
            else:
                args.max_batch_size_graph = 32
            if args.grapher_name == 'Embeddings':
                args.max_batch_size_graph = 4
            args.max_batch_size_train = 64
            args.lr_encoder = 5e-5
            args.decay_encoder = 1e-4
            args.decay_head = 1e-4
            args.batch_size = 32
            if args.trainer_name == 'MLM':
                args.lr_head = 1e-4
                args.epochs = 5
            if args.trainer_name == 'Supervised':
                args.lr_head = 0.001
                args.epochs = 3
        backbone_path = f'{PROJECT_DIR}/assets/language/transformers/{args.backbone_name}'
        model_kwargs = {'backbone_path': backbone_path}

    if args.model_name == 'RoBERTa':
        args.backbone_name = 'roberta-base-uncased'
        if auto_config:
            if args.grapher_name in DECOMPX_BASED_GRAPHERS:
                args.max_batch_size_graph = 2
            else:
                args.max_batch_size_graph = 32
            if args.grapher_name == 'Embeddings':
                args.max_batch_size_graph = 4
            args.max_batch_size_train = 64
            args.lr_encoder = 2e-5
            args.decay_encoder = 1e-4
            args.decay_head = 1e-4
            args.batch_size = 32
            if args.trainer_name == 'MLM':
                args.lr_head = 1e-4
                args.epochs = 5
            if args.trainer_name == 'Supervised':
                args.lr_head = 0.001
                args.epochs = 3
        backbone_path = f'{PROJECT_DIR}/assets/language/transformers/{args.backbone_name}'
        model_kwargs = {'backbone_path': backbone_path}

    if args.model_name == 'Tiny':
        if auto_config:
            args.epochs = 10
            args.batch_size = 32
            args.decay_encoder = 0
            args.decay_head = 0
            args.lr_encoder = 0.001
            args.lr_head = 0.001
            args.max_batch_size_graph = 512
            model_kwargs = {}

    if args.dataset_name == 'CoLa':
        args.max_length = 128
    if args.dataset_name == 'IMDb':
        args.max_length = 256

    if 'sFANS' in args.dataset_name:
        args.epochs = 8

    len_factor = 512 // args.max_length
    if args.aggregation_level == MODEL:
        args.tda_batch_size = 256 * len_factor
    if args.aggregation_level == LAYER:
        args.tda_batch_size = 64 * len_factor
    if args.aggregation_level == HEAD:
        args.tda_batch_size = 8 * len_factor

    assert args.model_name != 'GeneFormer' or not args.subset_hvg, "GeneFormer currently does not support HVG"
    assert args.eval_dataset_name == args.dataset_name or args.aggregation_level == DATASET
    assert (args.collection_level == DATASET and args.aggregation_level == DATASET) or args.moduler_name not in ['DITC', 'InformationBottleneck']
    assert (args.grapher_name is None and args.graph_path is None) or args.moduler_name not in ['DITC', 'InformationBottleneck']
    assert args.evaluator_name != 'Freeze' or args.aggregation_level != DATASET, "Freeze + DATASET deprecated as of 05/15/25"
    assert args.eval_dataset_name == args.dataset_name or args.aggregation_level == DATASET
    assert args.limit_samples_frac == 1 or args.aggregation_level == DATASET
    assert args.dataset_name != 'Wikipedia' or args.aggregation_level == DATASET
    assert args.max_batch_size_graph > 0, f"need more GPUs for graphing with {args.grapher_name} and {args.dtype_graph}"
    return model_kwargs