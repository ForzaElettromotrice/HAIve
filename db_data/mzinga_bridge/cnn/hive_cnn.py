import torch
import torch.nn as nn
import torch.optim as optim
from torch.functional import F
from masked_conv2d import MaskedConv2d

class HiveCNN(nn.Module):
    
    def __init__(self):
        super(HiveCNN, self).__init__()
        self.conv1 = MaskedConv2d(in_channels=1, out_channels=16)
        self.conv2 = nn.Conv2d(16, 32, kernel_size=3, padding=1)
        self.conv3 = nn.Conv2d(32, 64, kernel_size=3, padding=1)
        self.pool = nn.MaxPool2d(kernel_size=2, stride=2)
        self.fc1 = nn.Linear(64 * 7 * 7, 128)
        self.fc2 = nn.Linear(128, 64)
        self.fc3 = nn.Linear(64, 16)
        self.fc4 = nn.Linear(16, 1)
        
    def forward(self, x):
        x = F.tanh(self.conv1(x))
        x = self.pool(F.tanh(self.conv2(x)))
        x = self.pool(F.tanh(self.conv3(x)))
        
        x = x.view(x.size(0), -1)
        x = F.tanh(self.fc1(x))
        x = F.tanh(self.fc2(x))
        x = F.tanh(self.fc3(x))
        x = self.fc4(x)

        return x