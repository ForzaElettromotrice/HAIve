import os

"""
    Takes the x_file and the y_file and converts them into a single file
    with the following format:
    
    [((1, 2, 3), "W", "Q");((2, 3, 4), "B", "Q");...]1
    [((1, 2, 3), "W", "Q");((2, 3, 4), "B", "Q");...]0
"""

class Piece:
    
    def __init__(self, color: str, position: tuple[int, int, int], id: str):
        self.color = color
        self.position = position
        self.id = id
        
    def change_position(self, new_position: tuple[int, int, int]):
        self.position = new_position
        
    def get_position(self) -> tuple[int, int, int]:
        return self.position
        

class Collector:
    
    def __init__(self, first_move: str):
        first_piece = Piece(first_move[0], (0, 0, 0), first_move[1:])
        self.array: list[Piece] = [first_piece]
        self.position_chars: list[str] = [
            "-", "\\", "/"
        ]
        
    def add_piece(self, piece: Piece):
        self.array.append(piece)
        
    def get_piece(self, id: str, color: str) -> Piece:
        for piece in self.array:
            if piece.id == id and piece.color == color:
                return piece
        return None
    
    def manage_piece(self, mzinga_str: str):
        first_piece, second_piece = mzinga_str.split()[0:2]
        first_color = first_piece[0]
        first_name = first_piece[1:]
        first_piece: Piece = self.get_piece(first_name, first_color)
        if first_piece is None:
            first_piece = Piece(first_color, (0, 0, 0), first_name)
            self.add_piece(first_piece)
        
        if second_piece[0] in self.position_chars:
            second_color = second_piece[1]
            second_name = second_piece[2:]
            second_position = self.get_piece(second_name, second_color).get_position()
            position_change = (-2, 0, 0) if second_piece[0] == "-" else (
                                (-1, -1, 0) if second_piece[0] == "\\" else
                                (-1, 1, 0)
                            )
            
        elif second_piece[-1] in self.position_chars:
            second_color = second_piece[0]
            second_name = second_piece[1:-1]
            second_position = self.get_piece(second_name, second_color).get_position()
            position_change = (2, 0, 0) if second_piece[-1] == "-" else (
                                (1, 1, 0) if second_piece[-1] == "\\" else
                                (1, -1, 0)
                            )
            
        else:
            second_color = second_piece[0]
            second_name = second_piece[1:]
            second_position = self.get_piece(second_name, second_color).get_position()
            position_change = (0, 0, 1)
            
        new_position = (second_position[0] + position_change[0], second_position[1] + position_change[1], second_position[2] + position_change[2])
        first_piece.change_position(new_position)
        
    
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
        if os.path.exists(self.result_file):
            os.remove(self.result_file)
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
            collector: Collector = Collector(game_move[0])
            i: int = 1
            while True:
                if game_move[i] == "pass":
                    i += 1
                else:
                    new_move = game_move[i] + " " + game_move[i+1]
                    collector.manage_piece(new_move)
                    if i >= 8: 
                        self.dump_string(collector, label)
                    i += 2
                if i >= len(game_move):
                    break
                
    def dump_string(self, collector: Collector, label: str):
        with open(self.result_file, "a") as file:
            if label[-1] != "\n":
                label += "\n"
            file.write(str(collector) + label)
            
if __name__ == "__main__":
    converter = Converter("game_moves.txt", "labels.txt", "train_set.txt")
    converter.convert()