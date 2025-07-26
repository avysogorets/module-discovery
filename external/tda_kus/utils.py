import numpy as np
import json
from collections import defaultdict
from ...globals import DUMP_DIR


def cutoff_matrix(matrix, ntokens):
    """Return normalized submatrix of first n_tokens"""
    matrix = matrix[:ntokens, :ntokens]
    assert np.min(matrix) >= 0, "negative values in attention"
    assert np.max(matrix) <= 1, "some values exceed 1 in attention"
    # save matrix for inspection 1% of times:
    if np.random.choice([0,1], p=[0.99,0.01], size=1).item() > 0:
        np.save(f'{DUMP_DIR}/dump/{ntokens}.npy', matrix)
    # Modified by Artem Vysogorets
    # normalization has already been applied to attentions.
    # matrix /= matrix.sum(axis=1, keepdims=True)
    return matrix


def split_matrices_and_lengths(adj_matricies, ntokens, num_workers):
    splitted_ids = np.array_split(np.arange(ntokens.shape[0]), num_workers) 
    splitted = [(adj_matricies[ids], ntokens[ids]) for ids in splitted_ids]
    return splitted


def subprocess_wrap(queue, function, args):
    queue.put(function(*args))
    queue.close()