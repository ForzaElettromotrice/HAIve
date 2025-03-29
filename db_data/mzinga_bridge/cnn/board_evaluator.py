from hive_cnn import HiveCNN
from data_loader import Preprocessor

class MoveEvaluator():
    
    def __init__(self, checkpoint_file: str = "model_checkpoint.pth", model: HiveCNN = None):
        if model:
            self.model = model
        else:
            self.model = HiveCNN(checkpoint_file=checkpoint_file)
            self.model.load_model()
        
        self.model.eval()
        
    """
        Evaluate the given board state using the CNN model.
        
        @param board: The board state to evaluate in mzinga format.
    """
    def evaluate_mzinga(self, board: str) -> float:
        board = Preprocessor.mzinga_to_torch(board)
        
        result = self.model(board).item()
        return result