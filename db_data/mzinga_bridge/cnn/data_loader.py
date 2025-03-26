import torch
import torch.utils.data as data

import sys
import os
sys.path.append(os.path.abspath(os.getcwd()))

from db_data.mzinga_bridge.converter import Collector
from enum import Enum
import random

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

def vertical_shift(sample: str, possible_shifts: list[int] = [-5, -4, -3, -2, -1, 1, 2, 3, 4, 5]) -> str:
    sample, label = sample.split("]")[0:2]
    sample += "]"
    pieces = sample.strip('[]').split(';')
    new_sample = "["
    random_shift = random.choice(possible_shifts)
    for piece in pieces:
        if piece != '':
            coords, rest = piece.split(')', 1)
            x, y, z = map(int, coords[1:].split(','))
            y += random_shift
            new_sample += f'({x},{y},{z}){rest}'
            new_sample += ";"
    new_sample += "]"
    new_sample += label
    return new_sample

def horizontal_shift(sample: str, possible_shifts: list[int] = [-4, -3, -2, -1, 1, 2, 3, 4]) -> str:
    sample, label = sample.split("]")[0:2]
    sample += "]"
    pieces = sample.strip('[]').split(';')
    new_sample = "["
    random_shift = random.choice(possible_shifts)
    for piece in pieces:
        if piece != '':
            coords, rest = piece.split(')', 1)
            x, y, z = map(int, coords[1:].split(','))
            x += random_shift * 2
            new_sample += f'({x},{y},{z}){rest}'
            new_sample += ";"
    new_sample += "]"
    new_sample += label
    return new_sample

def diagonal_shift(sample: str, possible_shifts: list[int] = [-3, -2, -1, 0, 1, 2, 3]) -> str:
    sample, label = sample.split("]")[0:2]
    sample += "]"
    pieces = sample.strip('[]').split(';')
    new_sample = "["
    random_shift_x = random.choice(possible_shifts)
    random_shift_y = random.choice(possible_shifts)
    for piece in pieces:
        if piece != '':
            coords, rest = piece.split(')', 1)
            x, y, z = map(int, coords[1:].split(','))
            x += random_shift_x
            y += random_shift_y
            new_sample += f'({x},{y},{z}){rest}'
            new_sample += ";"
    new_sample += "]"
    new_sample += label
    return new_sample

def rotate_shift(sample: str, possible_rotations: list = [(1,-1),(-1,1),(-1,-1)]) -> str:
    sample, label = sample.split("]")[0:2]
    sample += "]"
    pieces = sample.strip('[]').split(';')
    new_sample = "["
    random_rotation = random.choice(possible_rotations)
    for piece in pieces:
        if piece != '':
            coords, rest = piece.split(')', 1)
            x, y, z = map(int, coords[1:].split(','))
            x *= random_rotation[0]
            y *= random_rotation[1]
            new_sample += f'({x},{y},{z}){rest}'
            new_sample += ";"
    new_sample += "]"
    new_sample += label
    return new_sample

def player_shift(sample: str) -> str:
    sample, label = sample.split("]")[0:2]
    sample += "]"
    pieces = sample.strip('[]').split(';')
    new_sample = "["
    for piece in pieces:
        if piece != '':
            coords, rest = piece.split(')', 1)
            x, y, z = map(int, coords[1:].split(','))
            color, piece = rest.split(',')[1:3]
            color = "w" if color == "b" else "b"
            new_sample += f'({x},{y},{z}),{color},{piece}'
            new_sample += ";"
    new_sample += "]"
    if label == "1":
        new_sample += "-1"
    elif label == "-1":
        new_sample += "1"
    else:
        new_sample += label
    return new_sample
    
class Preprocessor():
    
    @classmethod
    def mzinga_to_coord(cls, game: str) -> str:
        moves: list[str] = game.split(" ")
        i: int = 1
        collector: Collector = Collector(moves[0])
        while True:
            if i >= len(moves):
                break
            if moves[i] == "pass":
                i += 1
                continue
            else:
                collector.manage_piece(moves[i] + " " + moves[i+1])
                i += 2
                continue
        return str(collector)
    
    """
        Preprocess the string to the required format.
        
        @param matrix: The matrix to preprocess.
        @param size: The size of the output matrix.
    """
    @classmethod
    def preprocess(cls, matrix: str, size: tuple = (60, 60)):
        matrix = matrix.strip('[]')
        rows = matrix.split(';')
        
        matrix = torch.zeros((*size, len(Layer) + 1), dtype=torch.float16)
        if rows[-1] == '':
            rows = rows[:-1]
        for piece in rows:
            piece = piece.split(')')
            x, y, z = piece[0][1:].split(",")
            x, y, z = int(x), int(y), int(z)
            x += 30
            y += 30
            piece = piece[1].split(",")
            color = piece[1]
            id = piece[2]
            layer = get_layer(id, z)
            matrix[x, y, layer] = 1 if color == "w" else -1
        return matrix
    
    @classmethod
    def mzinga_to_torch(cls, game: str):
        game = cls.mzinga_to_coord(game)
        game = cls.preprocess(game)
        game = game.reshape((1, game.shape[0], game.shape[1], game.shape[2]))
        return game
    

class MatrixDataset(data.Dataset):
    
    def __init__(self, input_file: str, transform=None, size = (60, 60), data_augmentation: float = 1):
        self.input_file = input_file
        self.transform = transform
        self.preprocessor = Preprocessor
        with open(self.input_file, 'r') as f:
            self.data: list[str] = f.readlines()
        # self.data = [sample for sample in self.data if sample.split(']')[1] != '0']
        to_augment: int = int(len(self.data) * data_augmentation)
        to_vertical_shift = random.sample(self.data, int(to_augment / 6))
        for sample in to_vertical_shift:
            self.data.append(vertical_shift(sample))
        """
        to_horizontal_shift = random.sample(self.data, int(to_augment / 6))
        for sample in to_horizontal_shift:
            self.data.append(horizontal_shift(sample))
        to_diagonal_shift = random.sample(self.data, int(to_augment))
        for sample in to_diagonal_shift:
            self.data.append(diagonal_shift(sample))
        """
        to_rotate_shift = random.sample(self.data, int(to_augment))
        for sample in to_rotate_shift:
            self.data.append(rotate_shift(sample))
        to_player_shift = random.sample(self.data, int(to_augment / 4))
        for sample in to_player_shift:
            self.data.append(player_shift(sample))
        self.max_samples = len(self.data)
        self.size = size
        
    def preprocess(self, matrix):
        matrix = self.preprocessor.preprocess(matrix, self.size)
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