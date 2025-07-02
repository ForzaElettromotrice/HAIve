//
// Created by filippo on 15/06/25.
//

#include "heuristics.hpp"

uint_fast8_t getMovesSize(const Piece_t *moves)
{
    uint8_t size = 0;
    while (moves[size].id != NULLPIECE)
        size++;
    return size;
}

double mzingaHeuristic(Context_t *context)
{
    if (context->gameStatus == WHITE_WON)
        return static_cast<double>(Result::RESULT_WHITE_WON);
    if (context->gameStatus == BLACK_WON)
        return static_cast<double>(Result::RESULT_BLACK_WON);

    double result = 0;
    bool whiteTurn = context->curColor == WHITE;
    Piece_t *moves;
    if (context->curColor == BLACK)
        getMoves(context, &moves);
    else
    {
        context->curColor = static_cast<Colors_t>(context->curColor * -1);
        getMoves(context, &moves);
    }

    for (uint8_t i = B_QUEEN; i < NUM_PIECES; i++)
    {
        if (i == W_QUEEN)
        {
            context->curColor = static_cast<Colors_t>(context->curColor * -1);
            free(moves);
            getMoves(context, &moves);
        }
        const Position_t piecePos = context->idToPos[i];
        if (piecePos.z == -1)
            continue;
        const uint16_t mSize = getMovesSize(&moves[MMtA(i % 14, 0)]);
        HeuristicMetrics pieceMetric = getMetrics(static_cast<Pieces_t>(i));
        if ((whiteTurn && isBlack(i)) || (!whiteTurn && isWhite(i)))
            pieceMetric = pieceMetric.enemy();

        result += pieceMetric.inPlayWeight();

        if (piecePos.z < 5 && context->board[MtA(piecePos.z + 1, piecePos.y, piecePos.x)] != NULLPIECE)
            result += pieceMetric.isCoveredWeight();

        for (uint8_t j = 0; j < 6; j++)
        {
            const int_fast8_t newY = directions[j][0] + piecePos.y;
            const int_fast8_t newX = directions[j][1] + piecePos.x;

            const Pieces_t neighbor = context->board[MtA(piecePos.z, newY, newX)];
            if (neighbor != NULLPIECE)
            {
                if (isBlack(neighbor) && isBlack(i))
                    result += pieceMetric.friendlyNeighborWeight();
                else if (isWhite(neighbor) && isWhite(i))
                    result += pieceMetric.friendlyNeighborWeight();
                else
                    result += pieceMetric.enemyNeighborWeight();
            }
        }

        if (howManyAround(context, static_cast<Pieces_t>(i), true) + howManyAround(context, static_cast<Pieces_t>(i), false) > 2)
            result += pieceMetric.isPinnedWeight();

        const Pieces_t enemyQueen = isBlack(i) ? W_QUEEN : B_QUEEN;
        if (context->idToPos[enemyQueen].z == -1)
        {
            result += (pieceMetric.quietMoveWeight() * mSize);
            continue;
        }

        for (size_t j = 0; moves[MMtA(i % 14, j)].id != NULLPIECE; j++)
        {
            const Position_t newPos = moves[MMtA(i % 14, j)].position;
            const Position_t enemyQueenPos = context->idToPos[enemyQueen];
            if (abs(newPos.y - enemyQueenPos.y) + abs(newPos.x - enemyQueenPos.x) > 2)
            {
                result += pieceMetric.quietMoveWeight();
                continue;
            }
            if (abs(piecePos.y - enemyQueenPos.y) + abs(piecePos.x - enemyQueenPos.x) > 2)
                result += pieceMetric.noisyMoveWeight();
            else
                result += pieceMetric.quietMoveWeight();
        }
    }

    context->curColor = whiteTurn ? WHITE : BLACK;
    free(moves);

    const Pieces_t enemyQueen = whiteTurn ? B_QUEEN : W_QUEEN;

    result /= 5200000;
    // myVal
    if (context->idToPos[enemyQueen].z != -1)
    {
        // result += (0.17 * (howManyAround(context, enemyQueen, false) + howManyAround(context, enemyQueen, true)));
        int howMany = howManyAround(context, enemyQueen, false) + howManyAround(context, enemyQueen, true);
        result += (howMany * howMany * 0.032);
        result -= (howMany * 0.0286);
        result += 0.0179;
    }
    return result;
}

/*
    Fills x with some useful statistics about the context.
*/

void setHeuristicParams(const Context_t *context, torch::Tensor &x)
{
    // n_neigh_friendly + enemy  // moves (quiet/noisy)
    const size_t size = 2 * NUM_PIECES + 2 * NUM_PIECES;
    TORCH_CHECK(x.dim() == 1 && x.size(0) >= size, "Tensor x has incorrect shape");

    // For other tests: maybe adding positional+identitary embeddings of pieces.

    Piece_t *moves;
    getMoves(context, &moves);

    for (uint_fast8_t i = B_QUEEN; i < NUM_PIECES; i++)
    {
        x[i] = howManyAround(context, static_cast<Pieces_t>(i), true);
        x[i + NUM_PIECES] = howManyAround(context, static_cast<Pieces_t>(i), false);

        const Pieces_t enemyQueen = isBlack(i) ? W_QUEEN : B_QUEEN;
        uint16_t noisyMoves = 0, quietMoves = 0;
        if (context->idToPos[enemyQueen].z == -1)
        {
            for (uint_fast16_t j = 0; moves[MMtA(i % 14, j)].id != NULLPIECE; j++)
            {
                quietMoves++;
            }
        } else
        {
            for (size_t j = 0; moves[MMtA(i % 14, j)].id != NULLPIECE; j++)
            {
                const Position_t piecePos = context->idToPos[i];
                const Position_t newPos = moves[MMtA(i % 14, j)].position;
                const Position_t enemyQueenPos = context->idToPos[enemyQueen];

                if (abs(newPos.y - enemyQueenPos.y) + abs(newPos.x - enemyQueenPos.x) > 2)
                {
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
