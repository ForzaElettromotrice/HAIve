class DownloadedGame():

    def __init__(self, downloaded_file: str = "game.txt"):
        self.downloaded_file = downloaded_file

    def downloaded_game(self):
        with open(self.downloaded_file, "r") as file:
            game = file.read()
        moves = []
        for line in game.split("\n"):
            if line.strip():
                move = line.split(":")[1].strip()
                moves.append(move)
        moves = moves[1:]
        moves[0] = moves[0].split(".")[0].strip()
        game_string = " ".join(moves)
        return game_string