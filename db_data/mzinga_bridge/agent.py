import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "db_data/mzinga_bridge/cnn")))
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "db_data/mzinga_bridge")))

from cnn.board_evaluator import MoveEvaluator
from core.board import Board

class BoardManager:
    
    def __init__(self, status: list[str] = []):
        self.status = status
        self.move_evaluator = MoveEvaluator()
        self.move_evaluator.evaluate_mzinga(" ".join(self.status))
        
    def get_status(self):
        return ";".join(self.status)
        
    def evaluate(self):
        to_evaluate: str = " ".join(self.status)
        return self.move_evaluator.evaluate_mzinga(to_evaluate)
    
    def push(self, move):
        self.status.append(move)
        
    def pop(self):
        self.status.pop()

class Agent:
    def __init__(self, depth=2, board_status = [], game_string = ""):
        self.depth = depth
        self.board : BoardManager = BoardManager(status=board_status)

    def get_available_moves(self):
        is_white: bool = len(self.board.status) % 2 == 0
        num_turn: int = (len(self.board.status) // 2) + 1
        for_moves: Board = Board(
            "Base;InProgress;" + ("White[" if is_white else "Black[") + str(num_turn) + "];" + self.board.get_status()
        )
        return for_moves.valid_moves.split(";")
    
    def montecarlo_search(self, board: BoardManager, depth: int):
        if depth == 0:
            return self.board.evaluate(), None
        best_move = None
        best_score = float('-inf')
        moves = self.get_available_moves()
        for move in moves:
            board.push(move)
            score, _ = self.montecarlo_search(board, depth-1)
            score = -score
            board.pop()
            if score > best_score:
                best_score = score
                best_move = move
        return best_score, best_move
    
    def get_best_move(self):
        return self.montecarlo_search(self.board, self.depth)[1]
    
if __name__ == "__main__":
    
    game_state_string = "Base+MLP;InProgress;White[4];wB1;bG1 -wB1;wS1 wB1/;bQ \\bG1;wS2 \\wS1;bA1 /bG1;wQ wS2-;bA1 \\wS2"
    
    moves = game_state_string.split(";")
    moves = moves[3:]
    
    agent = Agent(board_status=moves, game_string=game_state_string)
    print(agent.get_best_move())