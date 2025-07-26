import torch


def optimized_transform(attention_probs, value_layer, headmixing_weight, chunk_size=32):
    d = headmixing_weight.shape[0]
    s = attention_probs.shape[-1]
    b = attention_probs.shape[0]
    tensor_kwargs = {
        'device': attention_probs.device,
        'dtype': attention_probs.dtype}
    weighted_layers = torch.zeros((b, s, s, d), **tensor_kwargs)
    for i in range(0, d, chunk_size):
        headmixing_weight_chunk = headmixing_weight[i:i+chunk_size,:,:]
        if len(value_layer.shape) == 4:
            transformed_layer = torch.einsum('bhsv,dhv->bhsd', value_layer, headmixing_weight_chunk)
            weighted_layer = torch.einsum('bhks,bhsd->bhksd', attention_probs, transformed_layer)
        if len(value_layer.shape) == 5:
            transformed_layer = torch.einsum('bhsqv,dhv->bhsqd', value_layer, headmixing_weight_chunk)
            weighted_layer = torch.einsum('bhks,bhsqd->bhkqd', attention_probs, transformed_layer)
        weighted_layers[...,i:i+chunk_size] = weighted_layer.sum(dim=1)
        del headmixing_weight_chunk, transformed_layer, weighted_layer
    return weighted_layers