import torch
import torch.nn as nn
import torch.optim as optim
from torch.functional import F
from masked_conv2d import MaskedConv2d

class HiveCNN(nn.Module):
    
    def __init__(self):
        super(HiveCNN, self).__init__()
        self.conv1 = MaskedConv2d(in_channels=12, out_channels=16)
        self.conv2 = nn.Conv2d(16, 32, kernel_size=3, padding=1, bias=True)
        self.conv3 = nn.Conv2d(32, 64, kernel_size=3, padding=1, bias=True)
        self.conv4 = nn.Conv2d(64, 128, kernel_size=3, padding=1, bias=True)
        self.pool = nn.MaxPool2d(kernel_size=2, stride=2)
        self.fc1 = nn.Linear(128 * 6 * 6, 128)
        self.dropout1 = nn.Dropout(0.25)
        self.fc2 = nn.Linear(128, 128)
        self.dropout2 = nn.Dropout(0.25)
        self.fc3 = nn.Linear(128, 32)
        self.dropout3 = nn.Dropout(0.25)
        self.fc4 = nn.Linear(32, 16)
        self.fc5 = nn.Linear(16, 1)
        
    def forward(self, x):
        x = F.tanh(self.conv1(x))
        x = self.pool(F.tanh(self.conv2(x)))
        x = self.pool(F.tanh(self.conv3(x)))
        x = self.pool(F.tanh(self.conv4(x)))
        
        x = x.reshape(x.size(0), -1)
        x = F.tanh(self.fc1(x))
        x = self.dropout1(x)
        x = F.tanh(self.fc2(x))
        x = self.dropout2(x)
        x = F.tanh(self.fc3(x))
        x = self.dropout3(x)
        x = F.tanh(self.fc4(x))
        x = self.fc5(x)

        return x