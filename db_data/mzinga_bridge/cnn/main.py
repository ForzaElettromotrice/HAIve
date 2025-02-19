import torch
from data_loader import MatrixDataset
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader
from hive_cnn import HiveCNN
import random

def get_loaders(input_file: str, train_size: float = 0.75) -> tuple[DataLoader, DataLoader]:
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


def train(model, train_loader, criterion, optimizer, num_epochs=40):
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
            
            predicted = outputs
            
        loss_value = total_loss / len(train_loader)
        # train_accuracy = test(model, train_loader, in_train=True)
        test_accuracy = test(model, test_loader, in_train=True)
        print(f"Epoch [{epoch+1}/{num_epochs}], Loss: {loss_value:.4f}, Test Accuracy: {test_accuracy:.4f}")


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
    

if __name__ == "__main__":
    
    train_loader, test_loader = get_loaders('train_set.txt')
    
    load_model: bool = False
    model = HiveCNN()
    optimizer = optim.Adam(model.parameters(), lr=0.001, weight_decay=1e-5)
    criterion = nn.MSELoss()

    if load_model:
        model.load_model(optimizer)
    
    train(model, train_loader, criterion, optimizer)
    test(model, test_loader)
    
    model.save_model(optimizer)

