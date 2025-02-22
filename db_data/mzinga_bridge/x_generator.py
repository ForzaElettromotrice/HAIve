from game_interface import GameInterface
from uhb_structs import *
from play_styles import *
import time

play_num: int       = 100
max_depth: int      = 5
out_file_name: str  = 'game_moves.txt'

if __name__ == '__main__':

    for i in range(0, play_num):
        game_interface = GameInterface()
        g_string: GameString = game_interface.newgame()
        start_time: float = time.time()
        looped = False
        while not g_string.game_state_string.is_game_ended():
            if time.time() - start_time > 1200: 
                looped = True
                break
            
            # move = game_interface.best_move_time(10)
            move = game_interface.best_move_depth(max_depth)
            
            if move is None or "err" in str(move):
                looped = True
                break
            
            g_string = game_interface.play(move)

        if looped:
            continue
        
        with open(out_file_name, 'a') as file:
            file.write(' '.join([str(move) for move in g_string.moves]) + '\n')