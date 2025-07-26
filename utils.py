import torch
import numpy as np
import pandas as pd
import logging
import torch.nn.functional as F
from typing import Union
from tqdm.auto import tqdm
from datetime import datetime
from .globals import *


def get_all_subclasses(cls):
    all_subclasses = []
    for subclass in cls.__subclasses__():
        all_subclasses.append(subclass)
        all_subclasses.extend(get_all_subclasses(subclass))
    return all_subclasses


def set_seeds(seed):
    np.random.seed(seed)
    torch.manual_seed(seed)
    torch.cuda.manual_seed(seed)


def get_specs_info(args):
    now = datetime.now()
    now_str = now.strftime("%d/%m/%Y %H:%M:%S")
    spec_names_dict = get_specs(args)
    spec_vals = {}
    max_len = 0
    for stage in spec_names_dict.keys():
        if len(spec_names_dict[stage]) == 0:
            continue
        spec_vals[stage] = get_spec_vals(args, spec_names_dict[stage])
        max_len_stage = max(len(spec_name) for spec_name in spec_names_dict[stage])
        max_len = max(max_len, max_len_stage)
    info_msg = f"\n[timestamp{' '*(max_len-9)}] {now_str}"
    for stage in spec_names_dict.keys():
        info_msg += f"\n[{stage.upper()+' '*(max_len-len(stage))}]"
        spec_names = spec_names_dict[stage]
        for i in range(len(spec_names_dict[stage])):
            info_msg += f"\n[{spec_names[i]+' '*(max_len-len(spec_names[i]))}] {spec_vals[stage][i]}"
    return info_msg


def get_specs(args):
    spec_names_general = [
        "seed",
        "dataset_name",
        "max_length"]
    if args.dataset_name == 'Switches':
        spec_names_general.append('num_groups')
        spec_names_general.append('num_tokens')
        spec_names_general.append('add_cls')
    if args.model_name == 'scGPT':
        spec_names_general.append('subset_hvg')
    spec_names_model = [
        "model_name",
        "trainer_name"]
    if args.model_name == 'Tiny':
        spec_names_model.append('num_layers')
    if args.model_name == 'scGPT':
        spec_names_model.append("scgpt_name")
    if args.model_name == 'GeneFormer':
        spec_names_model.append("geneformer_name")
    if args.trainer_name is not None:
        spec_names_model += [
        "lr_encoder",
        "lr_head",
        "decay_encoder",
        "decay_head",
        "epochs"]
    spec_names_graph = [
        "grapher_name",
        "aggregation_level",
        "collection_level",
        "layer",
        "process_before_aggregation",
        "dtype_graph"]
    if args.grapher_name in DECOMPX_BASED_GRAPHERS:
        spec_names_graph.append("mode")
        spec_names_graph.append("aggregation_in_layer")
        spec_names_graph.append("aggregation_in_encoder")
        spec_names_graph.append("mixing_in_layer")
        spec_names_graph.append("mixing_in_cls")
        spec_names_graph.append("include_biases")
    if args.grapher_name == 'MI':
        spec_names_graph.append("num_bins")
    if args.process_before_aggregation:
        spec_names_graph.append("remove_eye")
        spec_names_graph.append("remove_cls")
    if args.process_before_accumulation:
        spec_names_graph.append("remove_eye")
        spec_names_graph.append("remove_cls")
    if args.grapher_name == 'Embeddings':
        spec_names_graph.append("metric")
    if args.aggregation_level == DATASET:
        spec_names_graph.append("process_before_accumulation")
        spec_names_graph.append("limit_samples_frac")
    spec_names_normalize = [
        "frequency",
        "negatives",
        "correlation",
        "transpose",
        "zero_diag",
        "row_norm",
        "remove_eye",
        "sinkhorn",
        "uniform"]
    spec_names_module = [
        "moduler_name"]
    if args.moduler_name in ['DITC', 'InformationBottleneck']:
        spec_names_module.append('supervised')
    spec_names_evaluate = [
        "evaluator_name",
        "eval_dataset_name"]
    spec_names_dict = {
        "general": spec_names_general,
        "model": spec_names_model,
        "graph": spec_names_graph,
        "normalize": spec_names_normalize if args.grapher_name or args.graph_path else [],
        "module": spec_names_module,
        "evaluate": spec_names_evaluate}
    return spec_names_dict


def get_spec_vals(args, spec_names):
    spec_vals = []
    for spec_name in spec_names:
        spec_vals.append(str(getattr(args, spec_name)))
    return spec_vals


def get_fileid(args, stage=None):
    spec_names_dict = get_specs(args)
    general_fileid = '_'.join(get_spec_vals(args, spec_names_dict["general"]))
    if stage is None:
        return general_fileid
    if stage == ALL:
        fileid = '%'.join([
            general_fileid,
            '_'.join(get_spec_vals(args, spec_names_dict["model"])),
            '_'.join(get_spec_vals(args, spec_names_dict["graph"])),
            '_'.join(get_spec_vals(args, spec_names_dict["normalize"])),
            '_'.join(get_spec_vals(args, spec_names_dict["module"])),
            '_'.join(get_spec_vals(args, spec_names_dict["evaluate"]))])
        return fileid
    fileid = '_'.join([stage]+get_spec_vals(args, spec_names_dict[stage]))
    return fileid


def sinkhorn_normalizer(A: torch.Tensor, tolerance=1e-3, max_iters=100):
    """ Sinkhorn normalization algorithm that makes a tensor A to have
        *valid* rows and columns sum to 1. The tensor is required to
        have non-negative elements but it may have all-zero rows and/or
        columns. These indices are omitted from both row- and column-
        normalization.
    """
    logger = logging.getLogger(__name__)
    assert A.shape[-1] == A.shape[-2], "matrix is not square"
    assert (A >= 0).all(), "matrix has negative values"
    S = A.shape[-1]
    pos_row_mask = (A.sum(-1) > 0).int()
    pos_col_mask = (A.sum(-2) > 0).int()
    valid_mask = pos_col_mask * pos_row_mask
    valid_row_mask = valid_mask[...,None,:].repeat(*([1]*(len(A.shape)-2)+[S, 1]))
    valid_col_mask = valid_mask[...,:,None].repeat(*([1]*(len(A.shape)-2)+[1, S]))
    total_delta = tolerance + 1
    for i in tqdm(range(max_iters), desc='computing sinkhorn...'):
        row_norm = torch.sum(A * valid_row_mask, dim=-1, keepdim=True)
        row_norm[row_norm == 0] = 1
        A /= row_norm
        col_norm = torch.sum(A * valid_col_mask, dim=-2, keepdim=True)
        col_norm[col_norm == 0] = 1
        A /= col_norm
        row_sums = (A * valid_row_mask).sum(-1) * valid_mask
        col_sums = (A * valid_col_mask).sum(-2) * valid_mask
        row_delta = torch.norm(row_sums-valid_mask, p=1)
        col_delta = torch.norm(col_sums-valid_mask, p=1)
        total_delta = (abs(row_delta) + abs(col_delta)) * valid_mask.sum()
        if total_delta <= tolerance:
            break
        #if i == max_iters:
        #    logger.warning(f'sinkhorn did not converge: {total_delta:.5f} > {tolerance:.5f}')
    return A


def compute_correlations(
        attentions: torch.Tensor,
        masks: Union[None, torch.Tensor] = None,
        remove_eye: bool = False,
        remove_cls: bool = False):
    original_shape = attentions.shape
    if masks is None:
        masks = torch.ones(original_shape)
    assert masks.shape == attentions.shape
    S = original_shape[-1]
    attentions_flat = attentions.reshape(-1,S,S)
    masks_flat = masks.reshape(-1,S,S)
    correlations = []
    for i in range(attentions_flat.shape[0]):
        A = attentions_flat[i].T
        last_idx = torch.where(masks_flat[i,0] == 0)[0]
        if len(last_idx) > 0:
            last_idx = last_idx[0].item()
        else:
            last_idx = len(masks_flat[i])
        if remove_eye:
            A = A*(1-torch.eye(A.shape[-1]).to(A.device))
        A = A[:,:last_idx]
        if remove_cls:
            A = A[:,1:]
        corr = torch.corrcoef(A)
        correlations.append(corr)
    correlations = torch.stack(correlations)
    correlations = torch.nan_to_num(correlations, 0)
    correlations = correlations.reshape(original_shape)
    return correlations


def normalize(
    attentions,
    aggregation_level,
    two_counts=None,
    frequency=False,
    negatives=None,
    row_norm=False,
    sinkhorn=False,
    uniform=False,
    transpose=False,
    correlation=False,
    zero_diag=False,
    remove_eye=False):
    """ Normalizing the attention matrix based on the provided strategy.
        Note: you might want to mask out entries corresponding to padding
        tokens BEFORE applying normalization. If multiple strategies
        specified, they are performed in sequence.
    """
    # (for DATASET level only)
    # Range: R
    if frequency:
        err_msg = "frequency normalizer requires dataset-wide attention aggregation"
        assert aggregation_level == DATASET, err_msg
        two_counts[two_counts == 0] = 1
        attentions = attentions / two_counts
    # ~ structure change
    ######
    #two_counts[attentions == 0] = 0
    #attentions = two_counts
    ######
    # Range: R
    if transpose:
        dim_perm = list(range(len(attentions.shape)-2))+[-1,-2]
        attentions = attentions.permute(*dim_perm)
    # structure change:
    # Range: [-1,1]
    if correlation:
        attentions = compute_correlations(
            attentions=attentions,
            remove_eye=remove_eye,
            remove_cls=False)
    if zero_diag:
        S = attentions.shape[-1]
        mask = (1-torch.eye(S)).to(attentions.device)
        attentions = attentions * mask
    if negatives == 'clamp':
        attentions = torch.clamp(attentions, min=0)
    elif negatives == 'addmin':
        addnums = attentions.amin((-1,-2))
        attentions -= addnums
    elif negatives is None:
        pass
    else:
        raise NotImplementedError(f"neg handler {negatives} is undefined")
    # ~ normalization
    # Range: [0,1]
    if row_norm:
        assert attentions.amin() >= 0
        row_sums = attentions.sum(-1)
        row_sums[row_sums == 0] = 1.
        row_sums = row_sums[...,None]
        attentions = attentions / row_sums
    # ~ normalization
    # Range: [0,1]
    if sinkhorn:
        assert attentions.amin() >= 0
        attentions = sinkhorn_normalizer(attentions)
    # ~ normalization
    # Range: [0,1]
    if uniform:
        assert attentions.amin() >= 0
        tot_norm = attentions.amax((-1,-2))
        tot_norm = tot_norm[...,None,None]
        attentions /= tot_norm
    return attentions


def get_dtype(dtype_str):
    if dtype_str == 'float64':
        return torch.float64
    elif dtype_str == 'float32':
        return torch.float32
    elif dtype_str == 'float16':
        return torch.float16
    else:
        raise RuntimeWarning(f'dtype {dtype_str} is not supported')


def id2node_to_modules(id2node):
    modules_dict = {}
    for id, node in id2node.items():
        if node not in modules_dict:
            modules_dict[node] = []
        modules_dict[node].append(id)
    modules_list = []
    for node in modules_dict.keys():
        modules_list.append(modules_dict[node])
    return modules_list


def modules_to_id2node(modules):
    id2node = {}
    for i,module in enumerate(modules):
        for id in module:
            id2node[id] = i
    return id2node


def get_name2id_id2name():
    ensembl = pd.read_csv(GENE_MAP_FILEPATH)
    name2id = {name:id for name,id in zip(ensembl["gene_name"], ensembl["gene id"])}
    id2name = {}
    for gene_name, gene_id in name2id.items():
        id2name[gene_id] = gene_name
    return name2id, id2name


def prepare_pathways(filepath, token_dict):
    logger = logging.getLogger(__name__)
    if filepath.split('/')[-1] == 'pathways.csv':
        name2id,_ = get_name2id_id2name()
        token2i = token_dict
        i2token = {}
        for token, id in token2i.items():
            i2token[id] = token
        pathways = pd.read_csv(filepath)['gene_set']
        modules_true_named = [s.rstrip("']").lstrip("['").split("', '") for s in pathways]
        modules_true_eyed = []
        fail_genes = set()
        good_genes = set()
        for module_true_named in modules_true_named:
            module_true_eyed = []
            for gene_name in module_true_named:
                if gene_name not in name2id or name2id[gene_name] not in token2i:
                    fail_genes.add(gene_name)
                else:
                    good_genes.add(gene_name)
                    module_true_eyed.append(token2i[name2id[gene_name]])
            modules_true_eyed.append(module_true_eyed)
        logger.warning(f'[pathways gene conversion][OK {len(good_genes)}][FAIL {len(fail_genes)}]')
        return modules_true_eyed
    

def create_ids_mapping(data):
    vocab_ids_to_local_ids = torch.full((max(data.token_dict.values())+1,), float('nan'))
    for idx,id in enumerate(sorted(data.token_dict.values())):
        vocab_ids_to_local_ids[id] = idx
    vocab_ids_to_local_ids = vocab_ids_to_local_ids.long()
    return vocab_ids_to_local_ids
