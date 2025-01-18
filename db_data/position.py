from enum import Enum

class PieceType(Enum):
    NULLPIECE = 0,
    QUEEN = 1,
    PILLBUG = 2,
    LADYBUG = 3,
    MOSQUITO = 4,
    ANT = 5,
    GRASSHOPPER = 6,
    BEETLE = 7,
    SPIDER = 8
    
class Color(Enum):
    BLACK = 0,
    WHITE = 1

class Position:
    
    def __init__(self, id: PieceType, x: int, y: int, z: int, color: Color):
        self.id: PieceType = id
        self.x: int = x
        self.y: int = y
        self.z: int = z
        self.color: Color = color

    def __repr__(self):
        return f'({self.id},{self.x},{self.y},{self.z},{self.color})'
    
    def __str__(self):
        return self.__repr__()
    
    @staticmethod
    def parse(s: str) -> 'Position':
        s = s.strip('()')
        id_str, x, y, z, color = s.split(',')
        return Position(PieceType[id_str], int(x), int(y), int(z), Color[color])