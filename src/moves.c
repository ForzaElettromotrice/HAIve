//
// Created by f3m on 28/03/25.
//

#include "moves.h"

#include <stdbool.h>


/*
     1
    6 2
     0
    5 3
     4
0: (x, y)
1: (x, y-2)
2: (x+1, y-1)
3: (x+1, y+1)
4: (x, y+2)
5: (x-1, y+1)
6: (x-1, y-1)
*/
const int8_t directions[6][2] =
{
   //y   x
    {-2, 0}, //sopra
    {-1, 1}, //in alto a destra
    {1, 1}, //in basso a destra
    {2, 0}, //sotto
    {1, -1}, //in basso a sinistra
    {-1, -1} //in alto a sinistra
};

void queenMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
    //TODO
}


//[(id, mossa), (id,mossa)]