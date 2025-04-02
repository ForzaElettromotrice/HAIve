//
// Created by f3m on 28/03/25.
//

#include "moves.h"

#include <stdbool.h>

const int8_t directions[6][2] =
{
    {0, -2}, //sopra
    {-1, 1}, //in alto a destra
    {1, 1}, //in basso a destra
    {0, 2}, //sotto
    {1, -1}, //in basso a sinistra
    {-1, -1} //in alto a sinistra
};

void queenMoves(const Piece_t *piece, const Pieces_t *neighbors, Position_t *moves, const uint8_t *mSize)
{
    const Pieces_t id = piece->id;
    const int8_t ix = piece->position.x;
    const int8_t iy = piece->position.y;

    for (int8_t i = 0; i < 6; ++i)
    {
        if (neighbors[MtA(id, i)] != NULLPIECE)
            continue;

        if (neighbors[MtA(id, (i - 1) % 6)] != NULLPIECE && neighbors[MtA(id, (i + 1) % 6)] != NULLPIECE)
            continue;

        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        const Position_t move = {(int8_t) (ix + offX), (int8_t) (iy + offY), 0};
        moves[*mSize++] = move;
    }
}
