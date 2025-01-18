from game import Game

class DatabaseGenerator:
    
    def __init__(self, file_path: str):
        self.file_path: str = file_path
        self.data: list[Game] = []

    def generate(self, n: int) -> list[Game]:
        # Generate the database
        pass
        
    def save(self):
        with open(self.file_path, 'w') as f:
            for game in self.data:
                f.write(str(game) + '\n')