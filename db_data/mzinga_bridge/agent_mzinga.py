import sys
import os
import random

sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "db_data/mzinga_bridge/cnn")))
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "db_data/mzinga_bridge")))

from cnn.board_evaluator import MoveEvaluator
from game_interface import GameInterface

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
    
    def __init__(self, depth=2):
        self.depth = depth
        
    def run(self, against: str = "random", is_white: bool = True):
        game_interface = GameInterface()
        game_string = game_interface.newgame()
        random_move = random.choice(self.get_moves(game_interface))
        game_string = game_interface.play(random_move)
        board = BoardManager(status=[random_move])
        my_turn = not is_white
        
        while not game_string.game_state_string.is_game_ended():
            if my_turn:
                best_score, best_move = self.negamax_search(board, game_interface, self.depth)
                board.push(best_move)
                game_string = game_interface.play(best_move)
            else:
                random_move = random.choice(self.get_moves(game_interface))
                board.push(random_move)
                game_string = game_interface.play(random_move)
            my_turn = not my_turn
        return game_string.game_state_string.is_white_winner()
                
    def get_moves(self, game_interface: GameInterface) -> str:
        return [move.move for move in game_interface.validmoves()]
        
    def negamax_search(self, board: BoardManager, game_interface: GameInterface, depth: int):
        if depth == 0:
            return board.evaluate(), None
        best_move = None
        best_score = float('-inf')
        moves = self.get_moves(game_interface)
        for move in moves:
            board.push(move)
            score, _ = self.negamax_search(board, game_interface, depth-1)
            score = -score
            board.pop()
            if score > best_score:
                best_score = score
                best_move = move
        return best_score, best_move
    
if __name__ == '__main__':
    agent = Agent(depth=1)
    wins = 0
    plays = 0
    for i in range(100):
        is_win = agent.run()
        if is_win:
            wins += 1
        plays+=1
        print(f"Wins: {wins}, Plays: {plays}, Win rate: {wins/plays}")