import torch
import torch.nn as nn
import torch.optim as optim
from torch.functional import F
from masked_conv2d import MaskedConv2d

class HiveCNN(nn.Module):
    
    def __init__(self, checkpoint_file: str = "model_checkpoint.pth"):
        super(HiveCNN, self).__init__()
        
        self.checkpoint_file = checkpoint_file
        
        self.activation_func = F.relu
        
        def conv_block(in_channels, out_channels):
            return nn.Sequential(
                nn.Conv2d(in_channels, out_channels, kernel_size=3, padding=1, bias=True),
                nn.BatchNorm2d(out_channels),
                nn.ReLU()
            )
        
        self.layer1 = nn.Sequential(
            MaskedConv2d(in_channels=13, out_channels=24),
            nn.BatchNorm2d(24),
            nn.ReLU()
        )
        self.conv_layers = nn.Sequential(
            self.layer1,
            conv_block(24, 32),
            conv_block(32, 64),
            conv_block(64, 128),
            conv_block(128, 256)
        )
        
        self.pool = nn.MaxPool2d(kernel_size=2, stride=2)
        
        self.fc_layers = nn.Sequential(
            nn.Linear(128 * 512, 256),
            nn.ReLU(),
            nn.Dropout(0.2),
            nn.Linear(256, 128),
            nn.ReLU(),
            nn.Dropout(0.2),
            nn.Linear(128, 64),
            nn.ReLU(),
            nn.Linear(64, 1)
        )
        
    def forward(self, x):
        x = x.float()
        
        x = self.conv_layers(x)
        x = self.pool(x)
        
        x = x.reshape(x.size(0), -1)
        x = self.fc_layers(x)

        x = torch.tanh(x)

        return x
    
    def save_model(self, optimizer):
        checkpoint = {
            'model': self.state_dict(),
            'optimizer': optimizer.state_dict()
        }
        torch.save(checkpoint, self.checkpoint_file)
        
    def load_model(self, optimizer=None):
        checkpoint = torch.load(self.checkpoint_file)
        self.load_state_dict(checkpoint['model'])
        if optimizer:
            optimizer.load_state_dict(checkpoint['optimizer'])