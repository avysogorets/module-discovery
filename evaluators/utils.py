from sklearn import linear_model, preprocessing
from sklearn.naive_bayes import MultinomialNB
from sklearn.metrics import accuracy_score, precision_recall_fscore_support
from ..globals import LAYER, MODEL


def shapify(attentions, aggregation_level):
    if aggregation_level == MODEL:
        attentions = attentions[:,None,None,:,:]
    if aggregation_level == LAYER:
        attentions = attentions[:,:,None,:,:]
    return attentions


def train_LR(X, y, logger=None):
    Cs = [0.0001, 0.0005, 0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1, 2]
    max_iters = [10, 25, 50, 100, 500, 1000, 2000, 5000]
    assert len(X['train']) == len(y['train'])
    assert len(X['test']) == len(y['test'])
    scaler = preprocessing.StandardScaler()
    scaler.fit(X['train'])
    X['train'] = scaler.transform(X['train'])
    X['test'] = scaler.transform(X['test'])
    best_cls = None
    best_acc = 0
    best_parameters = None
    for C in Cs:
        for max_iter in max_iters:
            classifier = linear_model.LogisticRegression(
                C=C,
                penalty='l2',
                max_iter=max_iter,
                solver='lbfgs')
            classifier.fit(X['train'], y['train'])
            preds = classifier.predict(X['test'])
            acc = accuracy_score(y['test'], preds)
            if acc > best_acc:
                best_acc = acc
                best_cls = classifier
                best_parameters = (C, max_iter)
    if logger is not None:
        logger.info(f'best fit: {best_parameters} with acc.:  {best_acc:.4f}')
    metrics = {}
    for split in ['train', 'test']:
        preds = best_cls.predict(X[split])
        prfs = precision_recall_fscore_support(
            y[split],
            preds,
            average='macro')
        prec, recall, fscore,_ = prfs
        acc = accuracy_score(y[split], preds)
        metrics[split] = {
            'accuracy': acc,
            'recall': recall,
            'precision': prec,
            'fscore': fscore}
    return metrics


def train_NB(X, y, logger=None):
    alphas = [1e-4, 1e-3, 1e-2, 1e-1, 1]
    assert len(X['train']) == len(y['train'])
    assert len(X['test']) == len(y['test'])
    best_cls = None
    best_acc = 0
    best_parameters = None
    for alpha in alphas:
        classifier = MultinomialNB(alpha=alpha)
        classifier.fit(X['train'], y['train'])
        preds = classifier.predict(X['test'])
        acc = accuracy_score(y['test'], preds)
        if acc > best_acc:
            best_acc = acc
            best_cls = classifier
            best_parameters = (alpha,)
    if logger is not None:
        logger.info(f'best fit: {best_parameters} with acc.:  {best_acc:.4f}')
    metrics = {}
    for split in ['train', 'test']:
        preds = best_cls.predict(X[split])
        prfs = precision_recall_fscore_support(
            y[split],
            preds,
            average='macro')
        prec, recall, fscore,_ = prfs
        acc = accuracy_score(y[split], preds)
        metrics[split] = {
            'accuracy': acc,
            'recall': recall,
            'precision': prec,
            'fscore': fscore}
    return metrics