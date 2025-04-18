#include <logic.h>

int distance(Position_t *pos1, Position_t *pos2) {
	return abs(pos1->x - pos2->x) + abs(pos1->y - pos2->y);
}

GameStatus_t isGameEnded(Context_t* context) {

	// TODO: Check draw

	char nearQueen;
	nearQueen = howManyAround(context, W_QUEEN);
	if (nearQueen == 6)
		return WHITE_WON;
	nearQueen = howManyAround(context, B_QUEEN);
	if (nearQueen == 6)
		return BLACK_WON;
	return IN_PROGRESS;

}

// Tells how many pieces are around the given one
char howManyAround(Context_t* context, Pieces_t piece) {

	char nearAround = 0;
	Position_t* idToPos = context->idToPos;
	for (size_t pieceId = 0; pieceId < NUM_PIECES && nearAround < 6; pieceId++) {
		if (pieceId == piece)
			continue;
		Position_t* curPos = &idToPos[pieceId];
		if (curPos->x == -1)
			continue;
		if (curPos->z == idToPos[piece].z) {
			if (distance(curPos, &idToPos[piece]) == 2)
				nearAround++;
		}
	}
	// assert(nearAround between 1 and 6?)
	return nearAround;

}

/*
evaluate = (
    12 * opponent_queen_surround_ratio -          # Danger of opponent losing
    10 * own_queen_surround_ratio +               # Danger of you losing
    1.0 * own_mobility -						  !
    1.2 * opponent_mobility +					  !
    1.5 * own_pressure_on_opponent_queen -        ! # Pressure you're applying
    1.5 * opponent_pressure_on_own_queen +        ! # Threats you're facing
    2.0 * own_beetle_on_opponent_queen +          ! # Climbing the enemy queen is very strong
    1.0 * own_ant_freedom +                       ! # Ants are powerful if free
    0.8 * own_piece_count -						  
    0.5 * opponent_piece_count                    
)
*/

int32_t heuristicValue(Context_t *context, bool areWeWhite) {
	
	// TODO: other cases (signaled with a !)
	
	GameStatus_t gameStatus = context->gameStatus;
	if (gameStatus == WHITE_WON)
		return INT32_MAX;
	else if (gameStatus == BLACK_WON)
		return INT32_MIN;
	else if (gameStatus == DRAW || gameStatus == NOT_STARTED)
		return 0;

	int32_t value = 0;
	char ourPieces, displaced; ourPieces = displaced = 0;
	Position_t *idToPos = context->idToPos;
	for (size_t pieceId = 0; pieceId < NUM_PIECES; pieceId++) {
		if (idToPos[pieceId].x == -1)
			continue;
		displaced++;
		if (!areWeWhite && pieceId <= 13)
			ourPieces++;
		else if (areWeWhite && pieceId > 13)
			ourPieces++;
	}


	// opponent_queen_surround_ratio
	value += (12 * howManyAround(context, areWeWhite ? B_QUEEN : W_QUEEN));
	// own_queen_surround_ratio
	value -= (10 * howManyAround(context, areWeWhite ? W_QUEEN : B_QUEEN));
	// own_piece_count
	value += (int)(0.8 * ourPieces);
	// opponent_piece_count 
	value -= (int)(0.5 * (displaced - ourPieces));

	return value;
}