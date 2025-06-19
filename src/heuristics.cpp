//
// Created by filippo on 15/06/25.
//

#include "heuristics.h"

double mzingaHeuristic(const Context_t *context) {
    if (context->gameStatus == WHITE_WON)
        return static_cast<double>(Result::RESULT_WHITE_WON);
    if (context->gameStatus == BLACK_WON)
        return static_cast<double>(Result::RESULT_BLACK_WON);

    // TODO: Get moves both for WHite and Black

    double result = 0;
    bool whiteTurn = context->curColor == WHITE;
    Piece_t **moves;
    getMoves(context, &moves[0]);

    for (uint8_t i = B_QUEEN; i < NUM_PIECES; i++) {
        const Position_t piecePos = context->idToPos[i];
        const uint16_t mSize = getMovesSize(context, moves[i]);
        if (piecePos.z == -1)
            continue;
        HeuristicMetrics pieceMetric = getMetrics(static_cast<Pieces_t>(i));
        if (!whiteTurn)
            pieceMetric = pieceMetric.black();

        result += pieceMetric.inPlayWeight();

        if (piecePos.z < 5 && context->board[MtA(piecePos.z + 1, piecePos.y, piecePos.x)] != NULLPIECE)
            result += pieceMetric.isCoveredWeight();

        for (uint8_t j = 0; j < 6; j++) {
            const int_fast8_t newY = directions[j][0] + piecePos.y;
            const int_fast8_t newX = directions[j][1] + piecePos.x;

            const Pieces_t neighbor = context->board[MtA(piecePos.z, newY, newX)];
            if (neighbor != NULLPIECE) {
                if (isBlack(neighbor) && isBlack(i))
                    result += pieceMetric.friendlyNeighborWeight();
                else if (isWhite(neighbor) && isWhite(i))
                    result += pieceMetric.friendlyNeighborWeight();
                else
                    result += pieceMetric.enemyNeighborWeight();
            }
        }

        // How to check if isPinned?

        const Pieces_t enemyQueen = isBlack(i) ? W_QUEEN : B_QUEEN;
        if (context->idToPos[enemyQueen].z == -1)
            result += (pieceMetric.quietMoveWeight() * mSize);

        for (size_t j = 0; j < mSize; j++) {
            const Position_t newPos = moves[i][mSize].position;
            const Position_t enemyQueenPos = context->idToPos[enemyQueen];
            if (abs(newPos.y - enemyQueenPos.y) + abs(newPos.x - enemyQueenPos.x) > 2) {
                result += pieceMetric.quietMoveWeight();
                continue;
            }
            if (abs(piecePos.y - enemyQueenPos.y) + abs(piecePos.x - enemyQueenPos.x) > 2)
                result += pieceMetric.noisyMoveWeight();
            else
                result += pieceMetric.quietMoveWeight();
        }
    }

    result /= 10500000;
    // if (result > 1 || result < -1) printf("GOT EXCEEDING RESULT\n");
    return result;
}

/*
    Fills x with some useful statistics about the context.
*/

void setHeuristicParams(Context_t *context, torch::Tensor &x) {
    // n_neigh_friendly + enemy  // moves (quiet/noisy)
    size_t size = 2 * NUM_PIECES + 2 * NUM_PIECES;

    // For other tests: maybe adding positional+identitary embeddings of pieces.

    x.zero_();

    Piece_t **moves;
    getMoves(context, &moves[0]);

    for (uint_fast8_t i = B_QUEEN; i < NUM_PIECES; i++) {
        x[i] = howManyAround(context, static_cast<Pieces_t>(i), true);
        x[i + NUM_PIECES] = howManyAround(context, static_cast<Pieces_t>(i), false);

        const Pieces_t enemyQueen = isBlack(i) ? W_QUEEN : B_QUEEN;
        uint16_t noisyMoves = 0, quietMoves = 0;
        uint16_t mSize = getMovesSize(context, moves[i]);
        if (context->idToPos[enemyQueen].z == -1) {
            for (uint_fast16_t j = 0; j < mSize; j++) {
                quietMoves++;
            }
        } else {
            for (size_t j = 0; j < mSize; j++) {
                const Position_t piecePos = context->idToPos[i];
                const Position_t newPos = moves[i][mSize].position;
                const Position_t enemyQueenPos = context->idToPos[enemyQueen];

                if (abs(newPos.y - enemyQueenPos.y) + abs(newPos.x - enemyQueenPos.x) > 2) {
                    quietMoves++;
                    continue;
                }
                if (abs(piecePos.y - enemyQueenPos.y) + abs(piecePos.x - enemyQueenPos.x) > 2)
                    noisyMoves++;
                else
                    quietMoves++;
            }
        }
        x[i + 2 * NUM_PIECES] = quietMoves;
        x[i + 3 * NUM_PIECES] = noisyMoves;
    }
}
