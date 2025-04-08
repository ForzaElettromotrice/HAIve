import math
import random
import torch
import sys
import os

import uhb_structs
import game_interface

sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "db_data/mzinga_bridge/cnn")))
from cnn.data_loader import Preprocessor

class GameState:
    def __init__(self, game_string: uhb_structs.GameString):
        self.game_string = game_string
        self.tensor = None
        self.hash = None
        self.moves = None
        self.game = None
        
    def get_legal_moves(self):
        if self.moves is None:
            if self.game is None:
                self.game = game_interface.GameInterface()
            self.game.newgame_gs(self.game_string)
            self.moves = [move.move for move in self.game.validmoves()]
        return self.moves
    
    def is_terminal(self):
        return self.game_string.game_state_string.is_game_ended()
    
    def perform_move(self, move):
        if self.game is None:
            self.game = game_interface.GameInterface()
        self.game.newgame_gs(self.game_string)
        return GameState(self.game.play(move))
    
    def get_result(self):
        """Returns the result of the game from the perspective of the current player."""
        if self.game_string.game_state_string.is_white_winner():
            return 1
        elif self.game_string.game_state_string.is_black_winner():
            return -1
        else:
            return 0
        
    def to_tensor(self):
        if self.tensor is None:
            self.tensor = Preprocessor.mzinga_to_torch(" ".join([str(move) for move in self.game_string.moves]))
        return self.tensor
    
    def to_hash(self):
        if self.hash is None:
            self.hash = hash(self.tensor)
        return self.hash
    
    def is_white_turn(self) -> bool:
        return self.game_string.turn_string.is_white_turn()

class MCTSNode:
    def __init__(self, state, parent=None, prior=0):
        self.state: GameState = state
        self.parent: MCTSNode = parent
        self.children: list[MCTSNode] = []
        self.visits: int = 0
        self.value = 0
        self.prior = prior
    
    def is_fully_expanded(self):
        return len(self.children) == len(self.state.get_legal_moves())

    def best_child(self, exploration_weight=1.4):
        """Selects the best child node using UCT formula."""
        return max(self.children, key=lambda node: node.uct_score(exploration_weight))

    def uct_score(self, exploration_weight):
        if self.visits == 0:
            return float('inf')  # Prioritize unexplored nodes
        exploitation = self.value / self.visits
        exploration = exploration_weight * math.sqrt(math.log(self.parent.visits) / self.visits)
        return exploitation + exploration

class MCTS:
    def __init__(self, model, device, simulations=8):
        self.simulations = simulations
        self.model = model
        self.device = device

    def search(self, root_state: GameState, is_white: bool):
        root = MCTSNode(root_state)
        
        for _ in range(self.simulations):
            node = self.select(root)
            reward = self.simulate(node.state, is_white)
            self.backpropagate(node, reward)

        return root.best_child(0.2)

    def select(self, node: MCTSNode):
        """Find the best node to expand."""
        while not node.state.is_terminal():
            if not node.is_fully_expanded():
                return self.expand(node)
            node = node.best_child()
        return node

    def expand(self, node: MCTSNode):
        """Expand the tree by adding a new child node."""
        legal_moves = node.state.get_legal_moves()
        random.shuffle(legal_moves)
        for move in legal_moves:
            new_state = node.state.perform_move(move)
            if not any(child.state == new_state for child in node.children):
                child_node = MCTSNode(new_state, parent=node)
                node.children.append(child_node)
                return child_node
        return random.choice(node.children)

    def simulate(self, state: GameState, is_white: bool):
        """Simulate a game to the end from the current state."""
        if state.is_terminal():
            return state.get_result() if state.is_white_turn() else -state.get_result()
        state_tensor = state.to_tensor().to(self.device)
        with torch.no_grad():
            value = self.model(state_tensor).item()
        return value * (1 if state.is_white_turn() else -1)

    def backpropagate(self, node: MCTSNode, reward):
        """Propagate simulation results up the tree."""
        while node is not None:
            node.visits += 1
            node.value += reward
            reward = -reward  # Flip reward for the opponent
            node = node.parent