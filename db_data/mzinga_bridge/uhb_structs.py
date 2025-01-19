class GameTypeString:
    """
    A class representing the type of game.

    Attributes
    ----------

        mosquito (bool): If the mosquito expansion is included.
        ladybug (bool): If the ladybug expansion is included.
        pillbug (bool): If the pillbug expansion is included.

    """
    mosquito: bool
    ladybug: bool
    pillbug: bool

    def __init__(self, mosquito: bool = True, ladybug: bool = True, pillbug: bool = True):
        self.mosquito = mosquito
        self.ladybug = ladybug
        self.pillbug = pillbug

    def is_game_base_only(self) -> bool:
        """
        Determines if the game doesn't have any expansion.

        :return: True, if the game played is the base one.
        """
        return not self.mosquito and not self.ladybug and not self.pillbug

    def __str__(self):
        value = "Base"
        if self.mosquito or self.ladybug or self.pillbug:
            value += "+" + \
                     "M" if self.mosquito else "" + \
                                               "L" if self.ladybug else "" + \
                                                                        "P" if self.pillbug else ""
        return value

    def __repr__(self):
        return self.__str__()


class GameStateString:
    """
    A class representing the state of a game.

    Attributes
    ----------

        value (str): The string containing the status of the game.
    """
    NOT_STARTED: str = "NotStarted"
    IN_PROGRESS: str = "InProgress"
    DRAW: str = "Draw"
    WHITE_WINS: str = "WhiteWins"
    BLACK_WINS: str = "BlackWins"

    possible_values = [NOT_STARTED, IN_PROGRESS, DRAW, WHITE_WINS, BLACK_WINS]

    def __init__(self, value: str):
        if value not in self.possible_values:
            raise Exception("Incorrect value for GameStateString")
        self.value = value

    def is_game_not_started(self) -> bool:
        """
        Determines if the game isn't started yet.
        :return: True, if the game is yet to start.
        """
        return self.value == self.NOT_STARTED

    def is_game_in_progress(self) -> bool:
        """
        Determines if the game is in progress.
        :return: True, if the game is in progress.
        """
        return self.value == self.IN_PROGRESS

    def is_game_draw(self) -> bool:
        """
        Determines if the game has ended on a draw.
        :return: True, if the game has ended on a draw.
        """
        return self.value == self.DRAW

    def is_white_winner(self) -> bool:
        """
        Determines if the game has ended, and White is the winner.
        :return: True, if White has won.
        """
        return self.value == self.WHITE_WINS

    def is_black_winner(self) -> bool:
        """
        Determines if the game has ended, and Black is the winner.
        :return: True, if Black has won.
        """
        return self.value == self.BLACK_WINS

    def is_game_ended(self) -> bool:
        """
        Determines if the game has ended.
        :return: True, if the game has ended.
        """
        return not self.is_game_not_started() and not self.is_game_in_progress()

    def __str__(self):
        return self.value

    def __repr__(self):
        return self.value


class TurnString:
    """
    A class representing the turn of a game.

    Attributes
    ----------

        which_turn (str): The player (Black or White) which should play.
        num_turn (int): The turn number of the player.
    """
    BLACK: str = "Black"
    WHITE: str = "White"

    which_turn: str
    num_turn: int

    def __init__(self, which_turn: str, num_turn: int):
        if which_turn != self.BLACK and which_turn != self.WHITE:
            raise Exception("Incorrect value for player name for TurnString")
        self.which_turn = which_turn
        self.num_turn = num_turn

    def is_black_turn(self) -> bool:
        """
        Determines if the current turn is the black's.
        :return: True, if the current turn is the black's.
        """
        return self.which_turn == self.BLACK

    def is_white_turn(self) -> bool:
        """
        Determines if the current turn is the white's.
        :return: True, if the current turn is the white's.
        """
        return self.which_turn == self.WHITE

    def get_turn_number(self) -> int:
        """
        Returns the current turn number.
        :return:  The current turn number.
        """
        return self.num_turn

    def __str__(self):
        return self.which_turn + "[" + str(self.num_turn) + "]"

    def __repr__(self):
        return self.__str__()


class MoveString:
    """
    A class representing a move done by a player.

    Attributes
    ----------

        move (str): The move, in the UHP format.
    """

    move: str

    def __init__(self, move: str):
        self.move = move

    def by_black(self) -> bool:
        """
        Determines if the move was done by the Black player.
        :return: True, if the move was the black's.
        """
        return self.move[0] == 'b'

    def __str__(self):
        return self.move

    def __repr__(self):
        return self.move


class GameString:
    """
    A class representing a game.

    Attributes
    ----------

        game_type_string (GameTypeString): The type of the game.
        game_state_string (GameStateString): The current state of the game.
        turn_string (TurnString): The current turn of the game.
        moves (list[MoveString]): The history of all past moves.
    """
    game_type_string: GameTypeString
    game_state_string: GameStateString
    turn_string: TurnString
    moves: list[MoveString]

    def __init__(self, game_type_string: GameTypeString, game_state_string: GameStateString, turn_string: TurnString,
                 moves: list[MoveString]):
        self.game_type_string = game_type_string
        self.game_state_string = game_state_string
        self.turn_string = turn_string
        self.moves = moves

    def __str__(self):
        to_ret = ""
        to_ret += (str(self.game_type_string) + ";" + str(self.game_state_string) + ";" + str(self.turn_string) + ";")
        to_ret += ";".join([str(move) for move in self.moves])
        return to_ret

    def __repr__(self):
        return self.__str__()


def parse_turnstring(to_parse: str) -> TurnString:
    """
    Utility method.
    Parse a string to its respective TurnString representation.

    :param to_parse: The string to parse.
    :return: The TurnString parsed.
    """
    splitted = to_parse.split("[")
    if len(splitted) != 2:
        raise Exception("Wrong TurnString format")
    return TurnString(splitted[0], int(splitted[1].split("]")[0]))


def parse_gametypestring(to_parse: str) -> GameTypeString:
    """
    Utility method.
    Parse a string to its respective GameTypeString representation.

    :param to_parse: The string to parse.
    :return: The GameTypeString parsed.
    """
    if not to_parse.startswith("Base"):
        raise Exception("Failed parsing of GameTypeString")
    return GameTypeString(
        mosquito=to_parse.__contains__("M"),
        ladybug=to_parse.__contains__("L"),
        pillbug=to_parse.__contains__("P")
    )

def parse_movestring(to_parse: str) -> MoveString:
    """
    Utility method.
    Parse a string to its respective MoveString representation.

    :param to_parse: The string to parse.
    :return: The MoveString parsed.
    """

    return MoveString(to_parse)

def parse_game_string(to_parse: str) -> GameString:
    """
    Utility method.
    Parse a string to its respective GameString representation.

    :param to_parse: The string to parse.
    :return: The GameString parsed.
    """
    parts = to_parse.split(";")
    return GameString(
        parse_gametypestring(parts[0]),
        GameStateString(parts[1]),
        parse_turnstring(parts[2]),
        [MoveString(el) for el in parts[3:]]
    )
