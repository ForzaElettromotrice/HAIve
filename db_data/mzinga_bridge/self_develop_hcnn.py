import torch
import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "db_data/mzinga_bridge/cnn")))
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "db_data/mzinga_bridge")))

from cnn.data_loader import MatrixDataset
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader
from cnn.hive_cnn import HiveCNN
from cnn.board_evaluator import MoveEvaluator
from game_interface import GameInterface
import random
from mcts import MCTS, GameState, MCTSNode
from converter import Converter

def get_loaders(input_file: str, device, train_size: float = 0.75) -> tuple[DataLoader, DataLoader]:
    with open(input_file, 'r') as file:
        lines = [line for line in file.readlines() if line.strip()]

    random.shuffle(lines)
    split_index = int(len(lines) * train_size)
    train_lines = lines[:split_index]
    test_lines = lines[split_index:]

    with open('train_file.txt', 'w') as train_file:
        train_file.writelines(train_lines)

    with open('test_file.txt', 'w') as test_file:
        test_file.writelines(test_lines)

    train_dataset = MatrixDataset('train_file.txt')
    test_dataset = MatrixDataset('test_file.txt')
    
    train_loader = DataLoader(train_dataset, batch_size=32, shuffle=True)
    test_loader = DataLoader(test_dataset, batch_size=32, shuffle=True)

    return train_loader, test_loader


def train(model, train_loader, criterion, optimizer, num_epochs=5):
    model.train()
    
    for epoch in range(num_epochs):
        total_loss = 0
        
        for inputs, labels in train_loader:
            optimizer.zero_grad()
            
            outputs = model(inputs)
            outputs = outputs.view(-1)
            loss = criterion(outputs, labels.float())
            loss.backward()
            optimizer.step()
            
            total_loss += loss.item()
            
        loss_value = total_loss / len(train_loader)
        
        if (epoch + 1) % 5 == 0:
            test_accuracy = test(model, test_loader, in_train=True)
            print(f"Epoch [{epoch+1}/{num_epochs}], Loss: {loss_value:.4f}, Test Accuracy: {test_accuracy:.4f}")
        else:
            print(f"Epoch [{epoch+1}/{num_epochs}], Loss: {loss_value:.4f}")


def test(model, test_loader, in_train: bool = False):
    model.eval()
    correct = 0
    total = 0
    
    with torch.no_grad():
        for inputs, labels in test_loader:
            outputs = model(inputs)
            predicted = torch.where((outputs > -0.05) & (outputs < 0.05), torch.tensor(0.0), torch.sign(outputs))
            for i in range(len(predicted)):
                correct += 1 if predicted[i] == labels[i] else 0
            total += labels.size(0)
    
    accuracy = correct / total
    if in_train:
        return accuracy
    else:
        print(f"Test Accuracy: {accuracy:.4f}")
    
def play_game(mcts: MCTS, game_interface: GameInterface, game_string, draw: bool = True) -> list[str]:
    history = []
    state_hash = {}
    i = 0
    while not game_string.game_state_string.is_game_ended():
        
        if (i+1) % 5 == 0:
            print(f"Move {i+1}")
        if i >= 300:
            break
        game_state: MCTSNode = mcts.search(GameState(game_string), i % 2 == 0)
        i += 1
        
        if draw:
            hash = game_state.state.to_hash()
            if hash in state_hash:
                state_hash[hash] += 1
            else:
                state_hash[hash] = 1
            if state_hash[hash] >= 3:
                return history, game_string
        
        move = game_state.state.game_string.moves[-1]
        game_string = game_interface.play(move)
        history.append(str(move))
    return history, game_string

if __name__ == "__main__":
    
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    
    load_model: bool = True
    model = HiveCNN()
    model = model.to("cuda" if torch.cuda.is_available() else "cpu")
    criterion = nn.MSELoss()
    model.eval()
    
    model_enemy = HiveCNN()
    model_enemy = model_enemy.to("cuda" if torch.cuda.is_available() else "cpu")
    model_enemy.eval()
    
    optimizer = optim.AdamW(model.parameters(), lr=1e-5, weight_decay=1e-5)

    if load_model:
        model.load_model(optimizer)
        model_enemy.load_model()
        
    EPOCHS = 10
    PLAYS_FOR_EPOCH = 10
    
    for i in range(EPOCHS):
        for play in range(0 if i == 0 else PLAYS_FOR_EPOCH):
            game_interface = GameInterface()
            game_string = game_interface.newgame()
            mcts = MCTS(model, device)
            history, game_string = play_game(mcts, game_interface, game_string, draw=False)
            
            with open(f"x_{i}.txt", 'a') as file:
                file.write(' '.join(history) + '\n')
            with open(f"y_{i}.txt", 'a') as file:
                if game_string.game_state_string.is_black_winner():
                    file.write('-1\n')
                elif game_string.game_state_string.is_white_winner():
                    file.write('1\n')
                else:
                    file.write('0\n')
            print(f"Epoch {i} --- play {play} finished")
            
        converter = Converter(f"x_{i}.txt", f"y_{i}.txt", f"train_{i}.txt")
        converter.convert()
        train_loader, test_loader = get_loaders(f"train_{i}.txt", device)
        train(model, train_loader, criterion, optimizer)
        print(f"Epoch {i}'s training finished")
        
        wins = 0
        for play in range(PLAYS_FOR_EPOCH):
            game_interface = GameInterface()
            game_string = game_interface.newgame()
            mcts = MCTS(model, device)
            mcts_enemy = MCTS(model_enemy, device)
            turn = 0
            while not game_string.game_state_string.is_game_ended():
                if turn % 2 == 0:
                    move = mcts.search(GameState(game_string), True).state.game_string.moves[-1]
                else:
                    move = mcts_enemy.search(GameState(game_string), False).state.game_string.moves[-1]
                game_string = game_interface.play(move)
                turn += 1
            if game_string.game_state_string.is_white_winner():
                wins += 1
            print(f"Play {play} finished -- {wins*100/(play + 1)}% wins")
        print(f"Epoch {i}, wins: {wins/PLAYS_FOR_EPOCH}")
        if wins/PLAYS_FOR_EPOCH >= 0.55:
            model.save_model(optimizer)
            model_enemy.load_model()
        else:
            model_enemy.save_model()
            model.load_model(optimizer)
            
            
            
            
    
    """
    train(model, train_loader, criterion, optimizer)
    test(model, test_loader)
    
    model.save_model(optimizer)
    """