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

void queenMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
    //TODO
}


//[(id, mossa), (id,mossa)]
