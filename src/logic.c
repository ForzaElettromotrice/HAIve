#include <logic.h>

int distance(Position_t *pos1, Position_t *pos2) {
	return abs(pos1->x - pos2->x) + abs(pos1->y - pos2->y);
}

void copyContext(Context_t* source, Context_t* dest) {

	// Direct copies
	dest->turn = source->turn;
	dest->curColor = source->curColor;
	dest->gameStatus = source->gameStatus;
	dest->gameType = source->gameType;
	dest->lastMovedPiece = source->lastMovedPiece;
	dest->movesSize = source->movesSize;
	
	// Copy of pointers
	memcpy(dest->board, source->board, sizeof(Pieces_t) * BOARD_SIZE);
	memcpy(dest->moves, source->moves, sizeof(char) * source->movesSize);
	memcpy(dest->idToPos, source->idToPos, sizeof(Position_t) * NUM_PIECES);

}

// Tells how many pieces are around the given one
char howManyAround(Context_t* context, Pieces_t id) {

	char nearAround = 0;
	int8_t z = context->idToPos[id].z;
	int8_t y = context->idToPos[id].y;
	int8_t x = context->idToPos[id].x;
	for (int_fast8_t i = 0; i < 6; ++i)
	{
		const int_fast8_t newY = (int_fast8_t)(directions[i][0] + y);
		const int_fast8_t newX = (int_fast8_t)(directions[i][1] + x);

		if (context->board[MtA(z, newY, newX)] != NULLPIECE)
			nearAround++;
	}
	return nearAround;

}

GameStatus_t isGameEnded(Context_t* context)
{

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



inline bool isWin(Context_t* context) {
	return howManyAround(context, context->curColor == WHITE ? B_QUEEN : W_QUEEN) == 6;
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

float heuristicValue(Context_t *context, bool areWeWhite) {
	
	// TODO: other cases (signaled with a !)
	
	GameStatus_t gameStatus = context->gameStatus;
	if (gameStatus == WHITE_WON)
		return INT32_MAX;
	else if (gameStatus == BLACK_WON)
		return INT32_MIN;
	else if (gameStatus == DRAW || gameStatus == NOT_STARTED)
		return 0;

	int32_t value = 0;
	int32_t maxValue = 0;
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
	maxValue += (12 * 6);
	// own_queen_surround_ratio
	value -= (10 * howManyAround(context, areWeWhite ? W_QUEEN : B_QUEEN));
	maxValue -= (10 * 6);
	// own_piece_count
	value += (int)(0.8 * ourPieces);
	maxValue += (0.8 * NUM_PIECES / 2);
	// opponent_piece_count 
	value -= (int)(0.5 * (displaced - ourPieces));
	maxValue -= (0.5 * NUM_PIECES / 2);

	return value / (float)maxValue;
}

float negamax(Context_t* context, int depth, int maxDepth, bool isWhiteTurn, Piece_t* bestMove) {

	if (depth >= maxDepth) {
		return heuristicValue(context, isWhiteTurn);
	}

	// Trova i figli
	Piece_t* moves;
	int_fast8_t mSize = 0;
	getMoves(context, &moves, &mSize);
	float maxVal = -2, tmp;
	Piece_t curBestMove;

	for (int_fast8_t i = 0; i < mSize; i++) {
		Context_t newContext;
		initContext(&newContext);
		copyContext(context, &newContext);

		manageMove(newContext, &moves[i]);
		if ((tmp = negamax(newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove)) > maxVal) {
			maxVal = tmp;
			curBestMove = &moves[i];
		}

		cleanContext(newContext);
	}

	if (mSize == 0) {
		Context_t newContext;
		initContext(&newContext);
		copyContext(context, &newContext);

		manageMove(newContext, NULL);
		maxVal = negamax(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove);

		cleanContext(&newContext);
	}

	
	if (depth == 0) {
		*bestMove = curBestMove;
		return 0;
	}
	else {
		return -maxVal;
	}

}