import torch
import logging
from torch.utils.data import Subset
from sklearn.metrics import precision_recall_fscore_support
from .models.model_base import ModelBase


class Trainer:
    def __init__(self,
            trainer_name,
            model,
            data,
            lr_encoder,
            lr_head,
            batch_size,
            max_batch_size,
            decay_encoder,
            decay_head,
            epochs,
            num_workers=0,
            verbose=False):

        self.trainer_name = trainer_name
        self.real_batch_size = min(batch_size, max_batch_size)
        self.num_workers = num_workers
        self.model = model
        self.data = data
        self.epochs = epochs
        self.logger = logging.getLogger(__name__)
        self.skip_step = 1 + (batch_size // (max_batch_size+1))
        if isinstance(model, ModelBase):
            assert model.output_type == trainer_name
            params_groups = [{
                'params': self.model.output_heads[trainer_name].parameters(),
                'lr': lr_head,
                'weight_decay': decay_head}]
            params_groups.append({
                'params': self.model.encoder.parameters(),
                'lr': lr_encoder,
                'weight_decay': decay_encoder})
            self.optimizer = torch.optim.Adam(params_groups, lr=lr_head)
        else:
            self.optimizer = torch.optim.Adam(
                self.model.parameters(),
                lr=lr_head,
                weight_decay=decay_head)
        if epochs > 10:
            self.scheduler = torch.optim.lr_scheduler.StepLR(
                self.optimizer,
                step_size=epochs//3,
                gamma=0.2)
        self.verbose = verbose
        self.metrics_history = {'train': {}, 'test': {}}
            
    def get_dataloader(self, split, rand_perm=None):
        dataset = self.data.datasets[split]
        if rand_perm is not None and self.data.allow_perm:
            dataset = Subset(dataset, rand_perm)
        dataloader = self.data.get_dataloader(
            dataset=dataset,
            is_masked=self.trainer_name == 'MLM',
            batch_size=self.real_batch_size,
            num_workers=self.num_workers)
        return dataloader
        
    def train(self):
        rand_perm = torch.randperm(len(self.data.datasets['train']))
        dataloaders = {
            'train': self.get_dataloader('train', rand_perm=rand_perm),
            'test': self.get_dataloader('test') if self.data.has_test else None}
        for epoch in range(self.epochs):
            if self.verbose:
                self.log_progress(epoch, dataloaders)
                # naive early stopping for Tiny model
                # if self.metrics_history['test'][epoch]['acc'] > 0.99:
                #     break
            for step, inp in enumerate(dataloaders['train']):
                self.model.train()
                for key in inp.keys():
                    inp[key] = inp[key].to(self.model.device)
                    if 'float' in str(inp[key].dtype):
                        inp[key] = inp[key].to(self.model.dtype)
                _, loss = self.model(inp)
                loss.backward()
                if (step+1) % self.skip_step == 0:
                    self.optimizer.step()
                    self.optimizer.zero_grad()
            if self.epochs > 10:
                self.scheduler.step()
        if self.verbose:
            self.log_progress(self.epochs, dataloaders)

    def log_progress(self, epoch, dataloaders):
        info = f'[epoch: {epoch}]'
        with torch.no_grad():
            train_metrics = self.validate(dataloaders['train'])
            train_info = get_validation_info(train_metrics)
            info += '[train]' + train_info 
            if self.data.has_test:
                test_metrics = self.validate(dataloaders['test'])
                test_info = get_validation_info(test_metrics)
                info += '[test]' + test_info
            else:
                test_metrics = None
        self.logger.info(info)
        self.metrics_history['train'][epoch] = train_metrics
        self.metrics_history['test'][epoch] = test_metrics

    def validate(self, dataloader):
        self.model.eval()
        total_loss = 0
        total_acc = 0
        total_valid = 0
        ys, preds = [], []
        for inp in dataloader:
            for key in inp.keys():
                inp[key] = inp[key].to(self.model.device)
                if 'float' in str(inp[key].dtype):
                    inp[key] = inp[key].to(self.model.dtype)
            out, loss = self.model(inp)
            y = inp['label']
            num_valid = (y != -100).sum().item()
            total_loss += num_valid*loss.item()
            total_valid += num_valid
            if self.trainer_name == 'Supervised':
                pred = torch.argmax(out, dim=-1)
                preds.append(pred)
                ys.append(y)
                acc = torch.sum(pred == y.long())
                total_acc += acc.item()
        loss = total_loss / total_valid
        metrics = {'loss': loss}
        if self.trainer_name == 'Supervised':
            preds = torch.cat(preds).cpu()
            ys = torch.cat(ys).cpu()
            prec, recall, _,_ = precision_recall_fscore_support(ys, preds, average='macro')
            acc = total_acc / total_valid
            metrics['acc'] = acc
            metrics['prec'] = prec
            metrics['recall'] = recall
        return metrics


def get_validation_info(metrics):
    info = ""
    for key in metrics.keys():
        info += f'[{key}: {metrics[key]:.3f}]'
    return info

        
