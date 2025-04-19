//
// Created by f3m on 28/03/25.
//

#include "moves.h"

//TODO: rendere la dfs iterativa
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
inline bool isCovered(const Position_t *pos, const Pieces_t *board)
{
    return board[MtA(pos->z + 1, pos->y, pos->x)] != NULLPIECE;
}

void queenMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
    const int_fast8_t y = piece->position.y;
    const int_fast8_t x = piece->position.x;
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        if (board[MtA(0, newY, newX)] != NULLPIECE)
            continue;

        if (!canSlide(&piece->position, i, board))
            continue;

        const Piece_t move = {piece->id, {0, newY, newX}};
        moves[(*mSize)++] = move;
    }
}
void beetleMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
    const int_fast8_t z = piece->position.z;
    const int_fast8_t y = piece->position.y;
    const int_fast8_t x = piece->position.x;
    for (int_fast8_t j = -1; j < 2; ++j)
    {
        for (int_fast8_t i = 0; i < 6; ++i)
        {
            const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
            const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);


            //Sale nella posizione piu alta
            if (board[MtA(z, newY, newX)] != NULLPIECE)
            {
                int_fast8_t n = z;
                while (board[MtA(++n, newY, newX)] != NULLPIECE)
                {
                }
                const Position_t pos = {n, newY, newX};
                if (!canSlide(&pos, i, board))
                    continue;
                const Piece_t move = {piece->id, {n, newY, newX}};
                moves[(*mSize)++] = move;
                continue;
            }


            if (!canSlide(&piece->position, i, board))
                continue;

            //Scende alla posizione piu bassa
            int_fast8_t n;
            for (n = z; n > -1; --n)
            {
                if (n == 0)
                    break;
                if (board[MtA(n-1, newY, newX)] != NULLPIECE)
                    break;
            }

            const Piece_t move = {piece->id, {n, newY, newX}};
            moves[(*mSize)++] = move;
        }
    }
}
void grasshopperMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
    const int_fast8_t y = piece->position.y;
    const int_fast8_t x = piece->position.x;

    for (int_fast8_t i = 0; i < 6; i++)
    {
        int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        Pieces_t neighbor = board[MtA(0, newY, newX)];

        if (neighbor == NULLPIECE)
            continue;

        int_fast8_t n = 1;
        while (true)
        {
            newY = (int_fast8_t) (directions[i][0] * n + y);
            newX = (int_fast8_t) (directions[i][1] * n + x);
            neighbor = board[MtA(0, newY, newX)];

            if (neighbor == NULLPIECE)
                break;

            n++;
        }

        const Piece_t move = {piece->id, {0, newY, newX}};
        moves[(*mSize)++] = move;
    }
}
void pillbugMoves(const Piece_t *piece, const Pieces_t *board, const Pieces_t last, Piece_t *moves, uint_fast8_t *mSize)
{
    const int_fast8_t y = piece->position.y;
    const int_fast8_t x = piece->position.x;

    int_fast8_t sizeFree = 0;
    Position_t freeLocations[6];
    //Per muovere se stesso
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        if (board[MtA(0, newY, newX)] != NULLPIECE)
            continue;

        const Position_t free = {0, newY, newX};
        freeLocations[sizeFree++] = free;
        if (!canSlide(&piece->position, i, board))
            continue;

        const Piece_t move = {piece->id, free};
        moves[(*mSize)++] = move;
    }

    //Per muovere gli altri
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        const Pieces_t neighbor = board[MtA(0, newY, newX)];

        if (neighbor == NULLPIECE || neighbor == last)
            continue;

        for (int_fast8_t j = 0; j < sizeFree; ++j)
        {
            const Piece_t move = {neighbor, freeLocations[j]};
            moves[(*mSize)++] = move;
        }
    }
}
void ladybugMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
    const uint_fast8_t mStart = *mSize;
    const int_fast8_t y = piece->position.y;
    const int_fast8_t x = piece->position.x;
    //Primo passo
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY1 = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX1 = (int_fast8_t) (directions[i][1] + x);

        if (board[MtA(0, newY1, newX1)] == NULLPIECE)
            continue;

        //Climb
        int_fast8_t n1 = 0;
        while (board[MtA(++n1, newY1, newX1)] != NULLPIECE)
        {
        }
        const Position_t pos1 = {n1, newY1, newX1};
        if (!canSlide(&pos1, i, board))
            continue;

        //Secondo passo
        for (int_fast8_t j = 0; j < 6; ++j)
        {
            const int_fast8_t newY2 = (int_fast8_t) (directions[j][0] + newY1);
            const int_fast8_t newX2 = (int_fast8_t) (directions[j][1] + newX1);

            if (newX2 == newX1 && newY2 == newY1)
                continue;

            int_fast8_t n2;
            if (board[MtA(n1, newY2, newX2)] == NULLPIECE)
            {
                //Crawl
                const Position_t pos2 = {n1, newY2, newX2};
                if (!canSlide(&pos2, j, board))
                    continue;
                n2 = n1;
            } else
            {
                //Climb
                n2 = 0;
                while (board[MtA(++n2, newY2, newX2)] != NULLPIECE)
                {
                }
                const Position_t pos2 = {n2, newY1, newX1};
                if (!canSlide(&pos2, j, board))
                    continue;
            }

            //Terzo passo
            for (int_fast8_t k = 0; k < 6; ++k)
            {
                const int_fast8_t newY3 = (int_fast8_t) (directions[k][0] + newY2);
                const int_fast8_t newX3 = (int_fast8_t) (directions[k][1] + newX2);

                if (newX3 == newX2 && newY3 == newY2)
                    continue;

                if (board[MtA(n2, newY3, newX3)] != NULLPIECE)
                    continue;

                const Position_t pos2 = {n2, newY3, newX3};
                if (!canSlide(&pos2, k, board))
                    continue;

                //Fall
                int_fast8_t n3;
                for (n3 = n2; n3 > -1; --n3)
                {
                    if (n3 == 0)
                        break;
                    if (board[MtA(n3-1, newY3, newX3)] != NULLPIECE)
                        break;
                }
                if (n3 != 0)
                    continue;

                bool cnt = false;
                const Piece_t move = {piece->id, {n3, newY3, newX3}};
                //Controllo se già l'ho inserita
                for (uint_fast8_t h = mStart; h < *mSize; ++h)
                {
                    const Piece_t *move2 = &moves[h];
                    if (memcmp(&move, move2, sizeof(Piece_t)) == 0)
                    {
                        cnt = true;
                        break;
                    }
                }
                if (cnt)
                    continue;
                moves[(*mSize)++] = move;
            }
        }
    }
}
void spiderMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
    const uint_fast8_t mStart = *mSize;
    const int_fast8_t y = piece->position.y;
    const int_fast8_t x = piece->position.x;

    //Primo passo
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY1 = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX1 = (int_fast8_t) (directions[i][1] + x);

        if (board[MtA(0, newY1, newX1)] != NULLPIECE)
            continue;

        if (!canSlide(&piece->position, i, board))
            continue;

        //Secondo passo
        for (int_fast8_t j = 0; j < 6; ++j)
        {
            const int_fast8_t newY2 = (int_fast8_t) (directions[j][0] + newY1);
            const int_fast8_t newX2 = (int_fast8_t) (directions[j][1] + newX1);

            if (newY2 == newY1 && newX2 == newX1)
                continue;

            if (board[MtA(0, newY2, newX2)] != NULLPIECE)
                continue;

            const Position_t pos2 = {0, newY2, newX2};
            if (!canSlide(&pos2, j, board))
                continue;

            //Terzo passo
            for (int_fast8_t k = 0; k < 6; ++k)
            {
                const int_fast8_t newY3 = (int_fast8_t) (directions[k][0] + newY2);
                const int_fast8_t newX3 = (int_fast8_t) (directions[k][1] + newX2);

                if (newY3 == newY2 && newX3 == newX2)
                    continue;

                if (board[MtA(0, newY3, newX3)] != NULLPIECE)
                    continue;

                const Position_t pos3 = {0, newY3, newX3};
                if (!canSlide(&pos3, j, board))
                    continue;

                bool cnt = false;
                const Piece_t move = {piece->id, {0, newY3, newX3}};
                //Controllo se già l'ho inserita
                for (uint_fast8_t h = mStart; h < *mSize; ++h)
                {
                    const Piece_t *move2 = &moves[h];
                    if (memcmp(&move, move2, sizeof(Piece_t)) == 0)
                    {
                        cnt = true;
                        break;
                    }
                }
                if (cnt)
                    continue;
                moves[(*mSize)++] = move;
            }
        }
    }
}
void antMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize)
{
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
        if (!dfs(&positions[i], board, visited, true))
        {
            visited[i] = false;
            board[MtA(pos.z, pos.y, pos.x)] = i;
            continue;
        }
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
                pillbugMoves(&piece, board, last, *moves, mSize);
                break;
            case B_LADYBUG:
            case W_LADYBUG:
                ladybugMoves(&piece, board, *moves, mSize);
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
                grasshopperMoves(&piece, board, *moves, mSize);
                break;
            case B_BEETLE_1:
            case B_BEETLE_2:
            case W_BEETLE_1:
            case W_BEETLE_2:
                beetleMoves(&piece, board, *moves, mSize);
                break;
            case B_SPIDER_1:
            case B_SPIDER_2:
            case W_SPIDER_1:
            case W_SPIDER_2:
                spiderMoves(&piece, board, *moves, mSize);
                break;
        }
    }
}

// FIXME: La mossa è un Position_t? Speriamo
//[(id, mossa), (id,mossa)]
