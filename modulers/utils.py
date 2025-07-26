import torch
from tqdm.auto import tqdm 
from ..graphers.utils import filter_ids
from ..utils import create_ids_mapping


def create_Ps(data, supervised, device):
    vocab_ids_to_local_ids = create_ids_mapping(data)
    T = max(vocab_ids_to_local_ids)
    N = len(data.datasets['train'])
    counts = torch.zeros(N, T).to(device)
    dataset = data.datasets['train']
    for i in tqdm(range(N), desc='computing distributions...'):
        input_ids = dataset[i]['input_ids']
        input_ids = input_ids[dataset[i]['attn_masks'] > 0]
        local_ids = vocab_ids_to_local_ids[input_ids]
        counts[i] = torch.bincount(local_ids, minlength=T)
    if supervised:
        P_cond = torch.zeros(data.num_classes, T).to(device)
        y = [dataset[i]['label'] for i in range(len(dataset))]
        y = torch.LongTensor(y).to(device)
        P_cond.index_add_(0, y, counts)
    else:
        P_cond = counts
    P_marg = P_cond.sum(0)
    P_marg = P_marg.to(device)
    Z = P_marg[None,:].clone()
    Z[Z == 0] = 1
    P_cond /= Z
    P_marg /= P_marg.sum()
    good_local_ids, good_vocab_ids,_ = filter_ids(
        P_cond.cpu(),
        token_dict=data.token_dict,
        vocab_ids_to_local_ids=vocab_ids_to_local_ids,
        remove_isolated_rows=False,
        remove_isolated_cols=True)
    P_cond = P_cond[:,good_local_ids]
    P_marg = P_marg[good_local_ids]
    return P_cond, P_marg, good_vocab_ids