from game_interface import GameInterface
import random

class Stats:
    
    class IntWrapper:
        def __init__(self):
            self.value = 0
            self.num = 0
            self.avg = 0.0
        def __int__(self):
            return self.value
        def __str__(self):
            return str(self.value)
        def __repr__(self):
            return str(self.value)
        def get(self):
            return self.value
        def set(self, value: int):
            self.value = value
        def set_if_greater(self, value: int):
            self.num += 1
            self.avg = self.avg + (value - self.avg) / self.num
            if value > self.value:
                self.value = value
    class DictWrapper:
        def __init__(self):
            self.value = {}
        def __str__(self):
            return str(self.value)
        def __repr__(self):
            return str(self.value)
        def set_if_greater(self, key: str, value: int):
            if key not in self.value:
                self.value[key] = value
            else:
                self.value[key] = max(self.value[key], value)
        def __getitem__(self, key: str):
            return self.value.get(key, 0)
        def __setitem__(self, key: str, value: int):
            self.value[key] = value
    class Test:
        def __init__(self):
            self.value = None
        def __str__(self):
            if self.value is None:
                return ''
            return f'{' '.join(self.value[0])}\n->\n{' '.join(self.value[1])}'
        def __repr__(self):
            return str(self.value)
        def set(self, value: tuple[list[str], list[str]]):
            if self.value is None:
                self.value = value
        def get(self):
            return self.value
    class TestWrapper:
        def __init__(self, max: int = 10):
            self.value: list['Stats'.Test] = []
            self.max = max
        def __str__(self):
            return "\n\n\n".join([str(t) for t in self.value])
        def __repr__(self):
            return str(self.value)
        def set(self, value: tuple[list[str], list[str]]):
            if len(self.value) < self.max:
                self.value.append(value)
        def get(self):
            return self.value
        def __getitem__(self, index: int):
            return self.value[index]
    
    def __init__(self):
        self.game_interface = GameInterface()
        self.max_moves = self.IntWrapper()
        self.max_pieces_moves = self.DictWrapper()
        self.tests = self.TestWrapper(max=10)
    
    def fill_stats(self, game_string, moves: list[str]):
        self.max_moves.set_if_greater(len(moves))
        
        pieces_moves = {}
        for k in moves:
            piece_id = k.move[0:3].strip()
            pieces_moves[piece_id] = 1 if piece_id not in pieces_moves.keys() else pieces_moves[piece_id] + 1
        for k in pieces_moves.keys():
            self.max_pieces_moves.set_if_greater(k, pieces_moves[k])
        if random.random() < 0.01:
            self.tests.set(([str(m) for m in game_string.moves], [str(m) for m in moves]))
            
    def play(self, games: int = 10):
        for i in range(games):
            game_string = self.game_interface.newgame()
            while not game_string.game_state_string.is_game_ended():
                moves = self.game_interface.validmoves()
                self.fill_stats(game_string, moves)
                move = random.choice(moves)
                game_string = self.game_interface.play(move)
            print(f"Game {i+1} ended")
            self.print_stats()
            
    def print_stats(self):
        print(f"Max moves: {self.max_moves.get()} | Avg moves: {self.max_moves.avg:.2f}")
        print("--------------------")
        print("Max pieces moves:")
        for k in self.max_pieces_moves.value.keys():
            print(f"{k}: {self.max_pieces_moves[k]}")
        print("--------------------")

if __name__ == "__main__":
    stats = Stats()
    stats.play(2)
    print("Tests:")
    print(stats.tests)
    print("--------------------")
    print("End of tests")