import sys
import os
sys.path.append(os.path.abspath(os.getcwd()))
from db_data.mzinga_bridge.downloaded_games import DownloadedGame
from db_data.mzinga_bridge.converter import Collector
from db_data.mzinga_bridge.cnn.hive_cnn import HiveCNN
from db_data.mzinga_bridge.cnn.data_loader import Preprocessor

def mzinga_to_coord(game: str) -> str:
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
    
if __name__ == "__main__":
    move_string: str = DownloadedGame().downloaded_game()
    move_string = Preprocessor.mzinga_to_torch(move_string)
    
    model: HiveCNN = HiveCNN()
    model.load_model()
    model.eval()
    result = model(move_string).item()
    print(result)