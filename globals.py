ALL = -2
LAST = -1
AUTO = -1
PROJECT_DIR = '/scratch/amv458/ModuleDiscovery'
UTILS_DIR = '/scratch/amv458/utils'
PROJECT_DIR = '/Users/avysogorets/Documents/RockefellerUniversity/ModuleDiscovery'
UTILS_DIR = '/Users/avysogorets/Documents/RockefellerUniversity/utils'
DUMP_DIR = '/Users/avysogorets/Documents/RockefellerUniversity/ModuleDiscovery'
PAD = 0
EOS = 1
CLS = 2
MASK = 3
GENE_MAP_FILEPATH = PROJECT_DIR + '/assets/genetics/ensembl.csv'
DECOMPX_BASED_GRAPHERS = ['ALTI', 'GlobEnc', 'DecompX', 'NormBased']
DATASET = 0 # target token attribution is V x V (dataset-wide)
MODEL = 1 # target token attribution is N x S x S (model-wide per sample)
LAYER = 2 # target token attribution is N x L x S x S (layer-wide per sample)
HEAD = 3 # target token attribution is N x L x H x S x S (head-wide per sample)
NORM_ALTI = 'alti'
NORM_L2 = 'l2'
MAX_LEN = 10000
SPLIT_TRAIN = 0
SPLIT_TEST = 1
HALF = 0
ONE = 1

TOPOLOGICAL_FEATURES = ['s', 'e', 'v', 'c', 'b0b1']
BARCODE_FEATURES = [
    'h0_s', 
    'h0_e',
    'h0_t_d', 
    'h0_n_d_m_t0.75',
    'h0_n_d_m_t0.5',
    'h0_n_d_l_t0.25',
    'h1_t_b',
    'h1_n_b_m_t0.25',
    'h1_n_b_l_t0.95', 
    'h1_n_b_l_t0.70',  
    'h1_s',
    'h1_e',
    'h1_v',
    'h1_nb']
TEMPLATE_FEATURES = ['self', 'beginning', 'prev', 'next', 'ids']

# DG: None
# D: None/freq
# A: None/freq/rn + None/corr + (if corr: None/cls/eye) 
# C: 
# if signed: try different negatives settings

# BERT + AG-NEWS + NONE
#  [][s-, d-] Embeddings (EVERY LAYER and -2) PBAgg L2 [norm: NA/freq]
#  [][s+, d-] Embeddings (EVERY LAYER and -2) PBAgg cosine [norm: NA/freq, neg: NA/clamp]
#  [][s-, d-] Embeddings (EVERY LAYER and -2) L2 [norm: NA]
#  [][s+, d-] Embeddings (EVERY LAYER and -2) cosine [norm: NA, neg: NA/clamp]
#  [][s-, d+] DecompX (EVERY LAYER and -2) vector norm [norm: None/freq, corr: None/C/Ccls/Ceye]
#  None/None: OK
#  freq/None: OK
#  None/C: 64141737--64141748
#  freq/C: 64141807--64141820
#  None/Ccls: 
#  freq/Ccls:
#  None/Ceye: 64141824--64141835
#  freq/Ceye: 64141842--64141853
#  [][s-, d+] DecompX (EVERY LAYER and -2) rollout norm
#  [][s+, d-] DecompX (EVERY LAYER and -2) vector angle 
#  [][s-, d+] Raw (EVERY LAYER and -2)
#  [][s+, d-] Raw PBAcc (EVERY LAYER and -2)
#  [][s+, d-] Rollout PBAcc (EVERY LAYER and -2)
#  [][s-, d+] Rollout (EVERY LAYER and -2)

# BERT + SST2 + NONE
#   DecompX (EVERY LAYER and -2) vector norm
#   DecompX (EVERY LAYER and -2) rollout norm
#   DecompX (EVERY LAYER and -2) vector angle
#   Embeddings (EVERY LAYER and -2) L2
#   Embeddings (EVERY LAYER and -2) cosine
#   Embeddings (EVERY LAYER and -2) PBAgg cosine
#   Embeddings (EVERY LAYER and -2) PBAgg L2 (63953114-63953125) 63953116_2 63953117_0, 63953118, ...
#   Raw (EVERY LAYER and -2)
#   Raw PBAcc (EVERY LAYER and -2)
#   Rollout PBAcc (EVERY LAYER and -2)
#   Rollout (EVERY LAYER and -2)


