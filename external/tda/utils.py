import numpy as np
import json
from collections import defaultdict


def cutoff_matrix(matrix, ntokens):
    """Return normalized submatrix of first n_tokens"""
    matrix = matrix[:ntokens, :ntokens]
    matrix /= matrix.sum(axis=1, keepdims=True)
    return matrix


def split_matrices_and_lengths(adj_matricies, ntokens, num_workers):
    splitted_ids = np.array_split(np.arange(ntokens.shape[0]), num_workers) 
    splitted = [(adj_matricies[ids], ntokens[ids]) for ids in splitted_ids]
    return splitted


def subprocess_wrap(queue, function, args):
    queue.put(function(*args))
    queue.close()