import torch
import torch.nn as nn

class MaskedConv2d(nn.Module):
    def __init__(self, in_channels, out_channels, padding=4):
        super(MaskedConv2d, self).__init__()
        self.kernel_size = 5
        self.padding = padding
        
        self.conv = nn.Conv2d(in_channels, out_channels, kernel_size=self.kernel_size, padding=self.padding, bias=True)
        
        # MASK: To put to 0 inexistent elements of the board
        self.mask = torch.tensor([
            [1, 0, 1, 0, 1],
            [0, 1, 0, 1, 0],
            [1, 0, 1, 0, 1], 
            [0, 1, 0, 1, 0],
            [1, 0, 1, 0, 1]
        ], dtype=torch.float32).view(1, 1, self.kernel_size, self.kernel_size)
        
        # SHARED WEIGHTS: To allow rotational invariance
        self.shared_weight = nn.Parameter(torch.randn(1))
        self.center_weight = nn.Parameter(torch.randn(1))

    def forward(self, x):
        # Force SHARED WEIGHTS
        weight = self.shared_weight * torch.ones_like(self.conv.weight)
        weight[:, :, 2, 2] = self.center_weight

        # Apply MASK
        masked_weight = weight * self.mask.to(weight.device)

        self.conv.weight = nn.Parameter(masked_weight, requires_grad=True)

        return self.conv(x)
