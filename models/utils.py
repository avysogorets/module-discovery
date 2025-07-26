def translate_key(torch_key):
    tensor_key = torch_key.replace('layers', 'layer')
    tensor_key = tensor_key.replace('self_attn.Wqkv', 'attention.self')
    tensor_key = tensor_key.replace('self_attn.out_proj', 'attention.output.dense')
    tensor_key = tensor_key.replace('norm1', 'attention.output.LayerNorm')
    tensor_key = tensor_key.replace('linear1', 'intermediate.dense')
    tensor_key = tensor_key.replace('norm2', 'output.LayerNorm')
    tensor_key = tensor_key.replace('linear2', 'output.dense')
    return tensor_key
    

def torch2trans(torch_state_dict):
    trans_state_dict = {}
    for torch_key, tensor in torch_state_dict.items():
        tensor_key = translate_key(torch_key)
        if 'attention.self' in tensor_key:
            split_dim = tensor.shape[0] // 3
            tensor_path = '.'.join(tensor_key.split('.')[:-1])
            tensor_type = tensor_key.split('.')[-1]
            trans_state_dict[tensor_path + '.query.' + tensor_type] = tensor[0*split_dim:1*split_dim]
            trans_state_dict[tensor_path + '.key.' + tensor_type] = tensor[1*split_dim:2*split_dim]
            trans_state_dict[tensor_path + '.value.' + tensor_type] = tensor[2*split_dim:3*split_dim]
        else:
            trans_state_dict[tensor_key] = tensor
    return trans_state_dict


def prepend(state_dict, s):
    new_state_dict = {}
    for key, tensor in state_dict.items():
        new_state_dict[s + key] = tensor
    return new_state_dict