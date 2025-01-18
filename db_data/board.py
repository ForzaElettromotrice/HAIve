from position import Position

class Board:
    
    def __init__(self, state: list[Position]):
        self.current_state = state

    def __str__(self):
        return  "[" + ",".join([
                    str(position) for position in self.current_state
                ]) + "]"

    def __repr__(self):
        return self.__str__()
    
    @staticmethod
    def parse(s: str) -> 'Board':
        positions_str = s.strip('[]')
        return Board([
            Position.parse(position.strip()) for position in positions_str.split(',')
        ])