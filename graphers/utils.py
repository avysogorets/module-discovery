import numpy as np


def filter_ids(
        matrix, # has to be ? x T
        token_dict,
        vocab_ids_to_local_ids,
        remove_isolated_cols=True,
        remove_isolated_rows=True):
    # Should we remove special tokens too?
    bad_local_ids = set()
    if remove_isolated_rows:
        bad_local_ids_source = np.where(matrix.sum(1) == 0)[0].tolist()
        bad_local_ids = bad_local_ids.union(set(bad_local_ids_source))
    if remove_isolated_cols:
        bad_local_ids_target = np.where(matrix.sum(0) == 0)[0].tolist()
        bad_local_ids = bad_local_ids.union(set(bad_local_ids_target))
    good_local_ids = set(list(range(matrix.shape[1])))-bad_local_ids
    good_vocab_ids = []
    good_tokens = []
    for token, id in sorted(token_dict.items(), key=lambda x: x[1]):
        if vocab_ids_to_local_ids[id].item() in good_local_ids:
            good_vocab_ids.append(id)
            good_tokens.append(token)
    good_local_ids = sorted(list(good_local_ids))
    return good_local_ids, good_vocab_ids, good_tokens


def mask_pad(matrix, mask):
    # matrix shape B x ... x S
    mask = expand_mask(matrix, mask)
    matrix = matrix * mask
    matrix = matrix * mask.transpose(-2,-1)
    return matrix

def expand_mask(matrix, mask):
    repeats = [1]
    dims = matrix.shape[1:-1]
    for dim in dims:
        mask = mask[:,...,None,:]
        repeats.append(dim)
    repeats.append(1)
    mask = mask.repeat(repeats)
    return mask
