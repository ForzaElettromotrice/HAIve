from game_interface import GameInterface
import uhb_structs

label_file: str = "labels.txt"

"""
    For every game in the game_moves file, play the game and write the result to the label file.
"""

if __name__ == "__main__":
    
    with open("game_moves.txt", "r") as file:
        game_moves = file.readlines()
        
        with open(label_file, "r") as lf:
            labels = lf.readlines()
        
        N = len(game_moves) - len(labels)
        if N <= 0:
            raise ValueError("The game_moves file must have more rows than the label file")
        game_moves = game_moves[-N:]
    
    for game_move in game_moves:
        game_move = game_move.split()
        game_interface = GameInterface()
        g_string: uhb_structs.GameString = game_interface.newgame()
        game_state_string: uhb_structs.GameString = game_interface.play(game_move[0])
        i: int = 1
        while True:
            if game_move[i] == "pass":
                game_state_string = game_interface.play("pass")
                i += 1
            else:
                game_state_string = game_interface.play(game_move[i] + " " + game_move[i+1])
                i += 2
            if game_state_string.game_state_string.is_black_winner():
                with open(label_file, "a") as lf:
                    lf.write("-1\n")
                break
            elif game_state_string.game_state_string.is_white_winner():
                with open(label_file, "a") as lf:
                    lf.write("1\n")
                break
            elif game_state_string.game_state_string.is_game_draw():
                with open(label_file, "a") as lf:
                    lf.write("0\n")
                break
        
        