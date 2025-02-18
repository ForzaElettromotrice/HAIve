import torch
import torch.utils.data as data
from enum import Enum

class Layer(Enum):
    QUEEN = 0
    ANT = 1
    BEETLE1 = 2
    BEETLE2 = 3
    BEETLE3 = 4
    SPIDER = 5
    GRASSHOPPER = 7
    MOSQUITO1 = 8
    MOSQUITO2 = 9
    MOSQUITO3 = 10
    LADYBUG = 11
    PILLBUG = 12

def get_layer(id: str, position_z: int) -> int:
        if "Q" in id:
            return Layer.QUEEN.value
        if "A" in id:
            return Layer.ANT.value
        if "B" in id:
            return Layer.BEETLE1.value + position_z
        if "S" in id:
            return Layer.SPIDER.value
        if "G" in id:
            return Layer.GRASSHOPPER.value
        if "M" in id:
            return Layer.MOSQUITO1.value + position_z
        if "L" in id:
            return Layer.LADYBUG.value
        if "P" in id:
            return Layer.PILLBUG.value

class MatrixDataset(data.Dataset):
    
    def __init__(self, input_file: str, transform=None, size = (50, 50)):
        self.input_file = input_file
        self.transform = transform
        with open(self.input_file, 'r') as f:
            self.data: list[str] = f.readlines()
        self.max_samples = len(self.data)
        self.size = size
        
        
    def preprocess(self, matrix):
        matrix = matrix.strip('[]')
        rows = matrix.split(';')
        
        matrix = torch.zeros((*self.size, len(Layer)), dtype=torch.float16)
        if rows[-1] == '':
            rows = rows[:-1]
        for piece in rows:
            piece = piece.split(')')
            x, y, z = piece[0][1:].split(",")
            x, y, z = int(x), int(y), int(z)
            x += 25
            y += 25
            piece = piece[1].split(",")
            color = piece[1]
            id = piece[2]
            layer = get_layer(id, z)
            matrix[x, y, layer] = 1 if color == "w" else -1
            
        if self.transform:
            matrix = self.transform(matrix)
        return matrix
    
    def __len__(self):
        return self.max_samples
    
    def __getitem__(self, idx):
        sample = self.data[idx].strip().split(']')
        matrix = sample[0] + ']'
        label = int(sample[1])
        
        matrix = self.preprocess(matrix)
        return matrix, label