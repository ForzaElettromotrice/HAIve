import random
from uhb_structs import *
from game_interface import GameInterface

class TreeMoves:

    def __init__(self, status: GameStateString, move: MoveString, white_turn: bool):
        self.value = None
        self.status: GameStateString = status
        self.move: MoveString = move
        self.white_turn: bool = white_turn
        self.children = []

    def add_child(self, node):
        self.children.append(node)

    def get_value(self):
        if self.value is not None:
            return self.value
        if len(self.children) == 0:
            self.value = 0.5 if (not self.status.is_game_ended() or self.status.is_game_draw()) else (
                0 if self.status.is_black_winner() else 1
            )
        else:
            self.value = max([child.get_value() for child in self.children]) if self.white_turn else \
                min([child.get_value() for child in self.children])
        return self.value

def random_move(game_interface: GameInterface) -> MoveString:
    moves: list[MoveString] = game_interface.validmoves()
    return random.choice(moves)

def better_move(game_interface: GameInterface, white_playing: bool) -> MoveString:
    moves: list[MoveString] = game_interface.validmoves()
    for move in moves:
        g_string: GameString = game_interface.play(move)
        if g_string.game_state_string.is_white_winner():
            game_interface.undo()
            return move
        game_interface.undo()
    return random.choice(moves)

def rec_best_move_depth(game_interface: GameInterface, white_playing: bool, cur_node: TreeMoves, possible_depth: int):
    if possible_depth == 0 or cur_node.status.is_game_ended():
        return

    for move in game_interface.validmoves():
        g_string = game_interface.play(move)
        new_child = TreeMoves(g_string.game_state_string, move, white_playing)
        cur_node.add_child(
            new_child
        )
        if white_playing and new_child.status.is_white_winner():
            return
        elif not white_playing and new_child.status.is_black_winner():
            return
        game_interface.undo()

    for child in cur_node.children:
        game_interface.play(child.move)
        rec_best_move_depth(game_interface, not white_playing, child, possible_depth - 1)
        game_interface.undo()

    return

def best_move_depth(game_interface: GameInterface, white_playing: bool, tree_depth: int) -> MoveString:
    trees = [
        TreeMoves(GameStateString("InProgress"), move, white_playing) for move in game_interface.validmoves()
    ]
    for tree in trees:
        rec_best_move_depth(game_interface, white_playing, tree, tree_depth)
    trees.sort(key=lambda k: k.get_value())
    return trees[0].move
