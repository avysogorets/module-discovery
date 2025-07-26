import os
from .utils import batch_tokenize
from transformers import BertModel, RobertaModel, AutoTokenizer, AutoModelForCausalLM
from .utils import TokenizedDataset
from ...globals import SPLIT_TRAIN, SPLIT_TEST
os.environ["TOKENIZERS_PARALLELISM"] = "false"

tokenizer_kwargs = {
    'add_special_tokens': True,
    # this is inefficient but OK for my needs
    'padding': 'max_length',
    'truncation': True,
    'padding_side': 'right',
    'is_split_into_words': False,
    'return_special_tokens_mask': True,
    'return_attention_mask': True,
    'return_tensors': 'pt'}


def get_tokenizer_and_length(backbone_path):
    tokenizer = AutoTokenizer.from_pretrained(backbone_path)
    if 'roberta' in backbone_path:
        model = RobertaModel.from_pretrained(backbone_path)
    elif 'bert' in backbone_path:
        model = BertModel.from_pretrained(backbone_path)
    elif 'gpt' in backbone_path:
        model = AutoModelForCausalLM.from_pretrained(backbone_path)
        tokenizer.pad_token = tokenizer.eos_token
    else:
        raise NotImplementedError(f'unknown backbone {backbone_path}')
    max_pos_emb = model.config.max_position_embeddings
    return tokenizer, max_pos_emb

def get_dataset_from_df(df, tokenizer, max_length, split):
    sentence_data = []
    has_sent_0 = 'sentence_0' in df.columns
    has_sent_1 = 'sentence_1' in df.columns
    has_sent_pairs = has_sent_0 and has_sent_1
    if has_sent_pairs:
        sentences_0 = df['sentence_0'].tolist()
        sentences_1 = df['sentence_1'].tolist()
        for i in range(len(df)):
            sentence_data.append([sentences_0[i], sentences_1[i]])
    else:
        sentence_data = df['sentence_0'].tolist()
    inputs = batch_tokenize(
        tokenizer=tokenizer,
        texts=sentence_data,
        batch_size=512,
        max_length=max_length,
        **tokenizer_kwargs)
    labels = df['label'].tolist()
    for i in range(len(inputs)):
        inputs[i]['label'] = labels[i]
    if split == 'train':
        split_code = SPLIT_TRAIN
    elif split == 'test':
        split_code = SPLIT_TEST
    else:
        raise RuntimeError(f"split name {split} not recognized")
    dataset = TokenizedDataset(inputs, split_code=split_code)
    return dataset


def get_dataset_from_ds(ds, tokenizer, max_length, split):
    """ AKA get TokenizedDataset from datasets.Dataset
    """
    inputs = batch_tokenize(
        tokenizer=tokenizer,
        texts=ds['text'],
        batch_size=512,
        max_length=max_length,
        **tokenizer_kwargs)
    if split == 'train':
        split_code = SPLIT_TRAIN
    elif split == 'test':
        split_code = SPLIT_TEST
    else:
        raise RuntimeError(f"split name {split} not recognized")
    dataset = TokenizedDataset(inputs, split_code=split_code)
    return dataset