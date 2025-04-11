//
// Created by f3m on 28/03/25.
//

#include "moves.h"

//Suppongo che visited mi venga consegnato già con la posizione iniziale a True, first serve così faccio il calcolo dei visitati solo una volta
bool dfs(const Position_t *start, const Pieces_t *board, bool *visited, const bool first)
{
    const int_fast8_t y = start->y;
    const int_fast8_t x = start->x;
    for (uint_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (y + directions[i][0]);
        const int_fast8_t newX = (int_fast8_t) (x + directions[i][1]);

        const Pieces_t neighbor = board[MtA(0, newY, newX)];
        if (neighbor == NULLPIECE || visited[neighbor])
            continue;

        visited[neighbor] = true;
        const Position_t newPos = {0, newY, newX};
        dfs(&newPos, board, visited, false);
    }

    if (first)
    {
        for (int_fast8_t i = 0; i < 28; ++i)
            if (!visited[i])
                return false;
    }
    return true;
}
bool canSlide(const Position_t *pos, const int_fast8_t direction, const Pieces_t *board)
{
    const int_fast8_t z = pos->z;
    const int_fast8_t y = pos->y;
    const int_fast8_t x = pos->x;

    int_fast8_t newY = (int_fast8_t) (y + directions[(direction - 1) % 6][0]);
    int_fast8_t newX = (int_fast8_t) (x + directions[(direction - 1) % 6][1]);
    if (board[MtA(z, newY, newX)] == NULLPIECE)
        return true;

    newY = (int_fast8_t) (y + directions[(direction + 1) % 6][0]);
    newX = (int_fast8_t) (x + directions[(direction + 1) % 6][1]);

    return board[MtA(z, newY, newX)] == NULLPIECE;
}
bool isCovered(const Position_t *pos, const Pieces_t *board)
{
    return board[MtA(pos->z + 1, pos->y, pos->x)] != NULLPIECE;
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

        if (!canSlide(&piece->position, i, board))
            continue;

        const Piece_t move = {piece->id, {0, newY, newX}};
        moves[(*mSize)++] = move;
    }
}

void getMoves(Pieces_t *board, const Position_t *positions, const Colors_t color, const Pieces_t last, Piece_t **moves, uint_fast8_t *mSize)
{
    //TODO: in teoria le mosse totali possibili so un numero fisso, metteri quello come grandezza dell'array
    *moves = malloc(100 * sizeof(Piece_t));
    if (!*moves)
    {
        E_Print("malloc: %s\n", strerror(errno));
        return;
    }


    const Pieces_t start = color == WHITE ? 14 : 0;
    const Pieces_t end = start + 14;

    bool visited[28] = {};
    for (uint_fast8_t i = 0; i < 28; ++i)
    {
        if (positions[i].z == -1)
            visited[i] = true;
    }

    for (Pieces_t i = start; i < end; ++i)
    {
        Position_t pos = positions[i];
        // se era l'ultimo mosso
        if (last == i)
            continue;

        //se ha un pezzo sopra
        if (isCovered(&pos, board))
            continue;

        // se muovendosi spaccherebbe la board
        visited[i] = true;
        board[MtA(pos.z, pos.y, pos.x)] = NULLPIECE;
        if (!dfs(&positions[i], board, visited, true))
        {
            visited[i] = false;
            board[MtA(pos.z, pos.y, pos.x)] = i;
            continue;
        }
        board[MtA(pos.z, pos.y, pos.x)] = i;
        visited[i] = false;


        //genera le mosse
        const Piece_t piece = {i, positions[i]};
        switch (i)
        {
            case NULLPIECE:
                break;
            case B_QUEEN:
            case W_QUEEN:
                queenMoves(&piece, board, *moves, mSize);
                break;
            case B_PILLBUG:
            case W_PILLBUG:
                //pillbugMoves(&piece, board, *moves, mSize);
                break;
            case B_LADYBUG:
            case W_LADYBUG:
                //ladybugMoves(&piece, board, *moves, mSize);
                break;
            case B_MOSQUITO:
            case W_MOSQUITO:
                //mosquitoMoves(&piece, board, *moves, mSize);
                break;
            case B_ANT_1:
            case B_ANT_2:
            case B_ANT_3:
            case W_ANT_1:
            case W_ANT_2:
            case W_ANT_3:
                //antMoves(&piece, board, *moves, mSize);
                break;
            case B_GRASSHOPPER_1:
            case B_GRASSHOPPER_2:
            case B_GRASSHOPPER_3:
            case W_GRASSHOPPER_1:
            case W_GRASSHOPPER_2:
            case W_GRASSHOPPER_3:
                //grasshopperMoves(&piece, board, *moves, mSize);
                break;
            case B_BEETLE_1:
            case B_BEETLE_2:
            case W_BEETLE_1:
            case W_BEETLE_2:
                //beetleMoves(&piece, board, *moves, mSize);
                break;
            case B_SPIDER_1:
            case B_SPIDER_2:
            case W_SPIDER_1:
            case W_SPIDER_2:
                //spiderMoves(&piece, board, *moves, mSize);
                break;
        }
    }
}

//[(id, mossa), (id,mossa)]
