import subprocess

from uhb_structs import *

EXE_PATH = "./db_data/mzinga_bridge/MzingaEngine.exe"


class GameInterface:
    """
    A class simulating a game of Hive through an external program.

    Attributes
    ----------

        game_process (subprocess.Popen): The current process of the interactive game.
    """

    INFO_CMD = "info\n"
    NEWGAME_CMD = "newgame"
    OK_SIG = "ok\n"
    PLAY_CMD = "play"
    VALIDMOVES_CMD = "validmoves\n"
    UNDO_CMD = "undo"
    BESTMOVE_TIME_CMD = "bestmove time"
    BESTMOVE_DEPTH_CMD = "bestmove depth"

    def __init__(self):
        self.game_process = subprocess.Popen(
            [EXE_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        self.read_process_until_ok(send_data=False)

    def read_process_until_ok(self, send_data: bool = True, joiner_outs: str = "") -> str:
        """
        Reads from *stdout* until it finds an **OK_SIG**, which signals the end of an output.

        :param send_data: If it's required to return the read data from *stdout*.
        :param joiner_outs: How to join the lines read. Default: "".
        :return: The read data if *send_data* is True, otherwise an empty string.
        """
        ret_str = ""
        while True:
            line = self.game_process.stdout.readline()
            if line == self.OK_SIG:
                break
            if send_data:
                ret_str += (line + joiner_outs)
        self.game_process.stdout.flush()
        return ret_str.rstrip()

    def send_command(self, cmd: str, joiner_outs: str = "") -> str:
        """
        Sends a command to the process and returns the output of the process.

        :param cmd: The command to send to the process.
        :param joiner_outs: How to join the lines read from *stdout*.
        :return: The data read from *stdout*.
        """
        if cmd.endswith("\n"):
            self.game_process.stdin.write(cmd)
        else:
            self.game_process.stdin.write(cmd + "\n")
        self.game_process.stdin.flush()

        return self.read_process_until_ok(send_data=True, joiner_outs=joiner_outs)

    def info(self) -> str:
        """
        Runs the *info* command.
        From UHP: "The info command asks the engine to return its identifier string, optionally followed by a list of capabilities that the engine supports."

        :return: The result of the *info* command.
        """
        return self.send_command(self.INFO_CMD)

    def newgame_no_expansion(self) -> GameString:
        """
        Runs the *newgame* command without parameters.
        From UHP: "The newgame command asks the engine to start a new base game with no expansion pieces and should return a GameString.".

        :return: The GameString of the new game started.
        """
        ret = self.send_command(self.NEWGAME_CMD).rstrip()
        return parse_game_string(ret)

    def newgame_gts(self, game_type_string: GameTypeString) -> GameString:
        """
        Runs the *newgame* command, specifying the GameTypeString.
        From UHP: "[...] the engine should start a new game of the type specified."

        :param game_type_string: The GameTypeString of the new game.
        :return: The GameString of the new game started.
        """
        ret = self.send_command(self.NEWGAME_CMD + " " + str(game_type_string) + "\n")
        return parse_game_string(ret)

    def newgame_gs(self, game_string: GameString) -> GameString:
        """
        Runs the *newgame* command, specifying the GameString.
        From UHP: "[...] the engine should load the exact game as specified."

        :param game_string: The GameString that the engine should load.
        :return: The GameString of the new game started.
        """
        ret = self.send_command(self.NEWGAME_CMD + " " + str(game_string) + "\n")
        return parse_game_string(ret)
    
    def newgame(self) -> GameString:
        ret = self.send_command(self.NEWGAME_CMD + " Base+MLP\n")
        return parse_game_string(ret)

    def play(self, move: MoveString) -> GameString:
        """
        Plays a move on the game.
        From UHP: "The play command asks the engine to play the specified MoveString and should return the updated GameString if successful."

        :param move: The MoveString representing the move to play.
        :return: The GameString of the game after the move has been played.
        """
        ret: str = self.send_command(self.PLAY_CMD + " " + str(move) + "\n")
        return parse_game_string(ret)

    def pass_cmd(self) -> GameString:
        """
        Pass the turn.

        :return: The GameString of the game after passing the turn.
        """
        return self.play(parse_movestring("pass"))

    def validmoves(self) -> list[MoveString]:
        """
        Gets the possible moves that are deemed valid by the engine.

        :return: The list of MoveString of all the possible moves.
        """
        ret: str = self.send_command(self.VALIDMOVES_CMD)
        parts: list[str] = ret.split(";")
        moves: list[MoveString] = [parse_movestring(el) for el in parts]
        return moves

    def undo(self) -> GameString:
        """
        Goes back one move in the game.
        From UHP: "The undo command asks the engine to undo one or more previous moves and should return the updated GameString if successful."

        :return: The GameString of the game after the last move has been undo.
        """
        ret: str = self.send_command(self.UNDO_CMD + "\n")
        return parse_game_string(ret)

    def undo_num(self, num_moves: int) -> GameString:
        """
        Goes back *num_moves* in the game.

        :param num_moves: The number of moves to go back.
        :return: The GameString of the game after the last *num_moves* moves have been undo.
        """
        ret: str = self.send_command(self.UNDO_CMD + " " + str(num_moves) + "\n")
        return parse_game_string(ret)
    
    def best_move_time(self, seconds: int, minutes: int = 0, hours: int = 0) -> MoveString:
        """
        Asks the engine for the best move to play, given a maximum depth and time.

        :param white_turn: If it's white's turn.
        :param max_depth: The maximum depth to search.
        :param max_time: The maximum time to search.
        :return: The best move to play.
        """
        time_str = f"{hours:02}:{minutes:02}:{seconds:02}"
        ret: str = self.send_command(f"{self.BESTMOVE_TIME_CMD} {time_str}\n")
        return parse_movestring(ret)
    
    def best_move_depth(self, max_depth: int) -> MoveString:
        """
        Asks the engine for the best move to play, given a maximum depth.

        :param white_turn: If it's white's turn.
        :param max_depth: The maximum depth to search.
        :return: The best move to play.
        """
        ret: str = self.send_command(f"{self.BESTMOVE_DEPTH_CMD} {max_depth}\n")
        return parse_movestring(ret)
