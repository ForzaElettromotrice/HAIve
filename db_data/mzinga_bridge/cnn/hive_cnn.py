import torch
import torch.nn as nn
import torch.optim as optim
from torch.functional import F
from masked_conv2d import MaskedConv2d

class HiveCNN(nn.Module):
    
    def __init__(self, checkpoint_file: str = "model_checkpoint.pth"):
        super(HiveCNN, self).__init__()
        
        self.checkpoint_file = checkpoint_file
        
        self.activation_func = F.tanh
        
        self.conv1 = MaskedConv2d(in_channels=12, out_channels=16)
        self.conv2 = nn.Conv2d(16, 32, kernel_size=3, padding=1, bias=True)
        self.conv3 = nn.Conv2d(32, 64, kernel_size=3, padding=1, bias=True)
        self.conv4 = nn.Conv2d(64, 128, kernel_size=3, padding=1, bias=True)
        self.conv5 = nn.Conv2d(128, 256, kernel_size=3, padding=1, bias=True)
        self.pool = nn.MaxPool2d(kernel_size=2, stride=2)
        
        self.fc1 = nn.Linear(256, 256)
        self.dropout1 = nn.Dropout(0.2)
        self.fc2 = nn.Linear(256, 64)
        self.dropout2 = nn.Dropout(0.2)
        self.fc3 = nn.Linear(64, 16)
        self.fc4 = nn.Linear(16, 1)
        
    def forward(self, x):
        x = self.pool(self.activation_func(self.conv1(x)))
        x = self.pool(self.activation_func(self.conv2(x)))
        x = self.pool(self.activation_func(self.conv3(x)))
        x = self.pool(self.activation_func(self.conv4(x)))
        x = self.pool(self.activation_func(self.conv5(x)))
        
        x = x.reshape(x.size(0), -1)
        x = self.activation_func(self.fc1(x))
        x = self.dropout1(x)
        x = self.activation_func(self.fc2(x))
        x = self.dropout2(x)
        x = self.activation_func(self.fc3(x))
        x = self.activation_func(self.fc4(x))

        return x
    
    def save_model(self, optimizer):
        checkpoint = {
            'model': self.state_dict(),
            'optimizer': optimizer.state_dict()
        }
        torch.save(checkpoint, self.checkpoint_file)
        
    def load_model(self, optimizer):
        checkpoint = torch.load(self.checkpoint_file)
        self.load_state_dict(checkpoint['model'])
        optimizer.load_state_dict(checkpoint['optimizer'])