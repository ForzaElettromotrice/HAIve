downloaded_file = "game.txt"

class DownloadedGame():

    def downloaded_game():
        with open(downloaded_file, "r") as file:
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