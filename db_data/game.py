from board import Board

class Game:
    
    def __init__(self, boards: list[Board]):
        self.boards = boards

    def __str__(self):
        return ",".join([
            str(board) for board in self.boards
        ])

    def __repr__(self):
        return self.__str__()
    
    @staticmethod
    def parse(s: str) -> 'Game':
        return Game([
            Board.parse(board) for board in s.split(',')
        ])