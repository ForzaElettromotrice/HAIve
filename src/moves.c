//
// Created by f3m on 28/03/25.
//

#include "moves.h"


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

bool canSlide(const int_fast8_t z, const int_fast8_t y, const int_fast8_t x, const int_fast8_t direction, const Pieces_t *board)
{
    int_fast8_t offY = directions[(direction - 1) % 6][0];
    int_fast8_t offX = directions[(direction - 1) % 6][1];
    if (board[MtA(z, y + offY, x + offX)] == NULLPIECE)
        return true;

    offY = directions[(direction + 1) % 6][0];
    offX = directions[(direction + 1) % 6][1];

    return board[MtA(z, y + offY, x + offX)] == NULLPIECE;
}


void queenMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
    const int_fast8_t x = piece->position.x;
    const int_fast8_t y = piece->position.y;
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + x);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + y);

        if (board[MtA(0, newX, newY)] != NULLPIECE)
            continue;

        if (!canSlide(0, x, y, i, board))
            continue;

        const Piece_t move = {piece->id, {0, y, x}};
        moves[(*mSize)++] = move;
    }
}

void getMoves(const Pieces_t *board, const Position_t positions, Colors_t color, const Pieces_t last, Piece_t **moves, int8_t *mSize)
{
    //TODO: in teoria le mosse totali possibili so un numero fisso, metteri quello come grandezza dell'array
    *moves = malloc(100 * sizeof(Piece_t));
    if (!*moves)
    {
        E_Print("malloc: %s\n", strerror(errno));
        return;
    }

    //TODO: per ogni pezzo
    //      - vedere se può muoversi
    //      - generare le sue mosse
}

//[(id, mossa), (id,mossa)]
