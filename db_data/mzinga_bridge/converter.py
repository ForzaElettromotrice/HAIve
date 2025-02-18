import uhb_structs
from game_interface import GameInterface

class Piece:
    
    def __init__(self, color: str, position: tuple[int, int, int], id: str):
        self.color = color
        self.position = position
        self.id = id
        
    def change_position(self, new_position: tuple[int, int, int]):
        self.position = new_position

class Collector:
    
    def __init__(self):
        self.array = []
        
    def add_piece(self, piece: Piece):
        self.array.append(piece)
        
    def get_piece(self, id: str):
        for piece in self.array:
            if piece.id == id:
                return piece
        return None
    
    def __str__(self) -> str:
        to_ret: str = "["
        for piece in self.array:
            to_ret += str(piece.position) + "," + piece.color + "," + piece.id + ";"
        to_ret += "]"
        return to_ret

class Converter:
    
    def __init__(self, mzinga_file: str, label_file: str, result_file: str):
        self.mzinga_file = mzinga_file
        self.label_file = label_file
        self.result_file = result_file
        with open(self.mzinga_file, "r") as file:
            mzinga_lines = file.readlines()
            
        with open(self.label_file, "r") as lf:
            label_lines = lf.readlines()
            
        if len(mzinga_lines) != len(label_lines):
            raise ValueError("The mzinga_file and label_file must have the same number of rows")
        
    def convert(self):
        with open(self.mzinga_file, "r") as file:
            game_moves = file.readlines()
            
        with open(self.label_file, "r") as lf:
            labels = lf.readlines()
            
        for game_move, label in zip(game_moves, labels):
            game_move = game_move.split()
            moves = [game_move[0]]
            i: int = 1
            while True:
                self.from_mzinga_to_board(moves)
                if game_move[i] == "pass":
                    moves.append("pass")
                    i += 1
                else:
                    moves.append(game_move[i] + " " + game_move[i+1])
                    i += 2
                if i >= len(game_move):
                    break
                
    def from_mzinga_to_board(self, moves: list[str]):
        collector: Collector = Collector()
        # TODO