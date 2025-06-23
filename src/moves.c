//
// Created by f3m on 18/06/25.
//

#include <stdio.h>
#include <stdint.h>
#include <enums.h>
#include <string.h>
#include <utils.h>
#include <hashmap.h>
#include <pthread.h>
#include <moves.h>

#include "logger.h"

typedef struct ThreadArgs
{
    const Context_t *context;
    Piece_t **moves;
} ThreadArgs_t;

Pieces_t chooseStartingPoint(const Pieces_t toMove, const Colors_t color, const Position_t *idToPos)
{
    if (toMove != B_QUEEN && toMove != W_QUEEN)
        return color == WHITE ? W_QUEEN : B_QUEEN;
    for (int i = 0; i < 28; ++i)
    {
        if (idToPos[i].z != -1 && i != toMove)
            return i;
    }
    //Unreachable
    logE(stderr, "Impossible to reach!\n");
    return -1;
}

void allocateMoves(Piece_t **moves)
{

    //TODO: fare una versione che alloca il giusto per ogni pezzo e vedere quale va piu veloce
    //Allocazione delle mosse
    for (int i = 0; i < MOVES_ARRAYS; ++i)
    {
        //FIXME: 30 è provvisorio, un valore più preciso potrebbe fare più comodo
        moves[i] = malloc(30 * sizeof(Position_t));
        memset(moves[i], 0xff, 30 * sizeof(Position_t));
    }
}

void freeMoves(Piece_t ***moves) {

    for (int i = 0; i < MOVES_ARRAYS; ++i)
    {
        free(moves[0][i]);
    }
    free(moves[0]);

}

bool canSlide(const Position_t *pos, const int_fast8_t direction, const Pieces_t *board)
{
    const int_fast8_t z = pos->z;
    const int_fast8_t y = pos->y;
    const int_fast8_t x = pos->x;

    int_fast8_t newY = (int_fast8_t) (y + directions[(direction + 5) % 6][0]);
    int_fast8_t newX = (int_fast8_t) (x + directions[(direction + 5) % 6][1]);
    if (board[MtA(z, newY, newX)] == NULLPIECE)
        return true;

    newY = (int_fast8_t) (y + directions[(direction + 1) % 6][0]);
    newX = (int_fast8_t) (x + directions[(direction + 1) % 6][1]);

    return board[MtA(z, newY, newX)] == NULLPIECE;
}
bool hasNeighbor(const Piece_t *piece, const Pieces_t *board)
{
    const Pieces_t id = piece->id;
    const int_fast8_t y = piece->position.y;
    const int_fast8_t x = piece->position.x;
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        const Pieces_t neighbor = board[MtA(0, newY, newX)];

        if (neighbor != id && neighbor != NULLPIECE)
            return true;
    }
    return false;
}
bool dfs(const Position_t *start, const Context_t *context, bool *visited, const bool first)
{
    const Pieces_t *board = context->board;
    const Position_t *positions = context->idToPos;

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
        dfs(&newPos, context, visited, false);
        if (first)
            break;
    }

    if (first)
    {
        for (int_fast8_t i = 0; i < 28; ++i)
            if (!visited[i] && positions[i].z != -1)
                return false;
    }
    return true;
}

void *addMoves(void *arguments)
{
    const ThreadArgs_t *args = (ThreadArgs_t *) arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[14];

    const Pieces_t start = context->curColor == WHITE ? 14 : 0;
    const Pieces_t end = start + 14;


    uint8_t addSize = 0;
    uint8_t checkSize = 0;
    uint8_t toAdd[14];
    uint8_t toCheck[14];
    for (uint_fast8_t i = start; i < end; ++i)
    {
        if (context->idToPos[i].z == -1)
        {
            toAdd[addSize++] = i;
            switch (i)
            {
                case B_ANT_1:
                case W_ANT_1:
                case W_GRASSHOPPER_1:
                case B_GRASSHOPPER_1:
                    i += 2;
                    break;
                case B_ANT_2:
                case W_ANT_2:
                case W_GRASSHOPPER_2:
                case B_GRASSHOPPER_2:
                case W_BEETLE_1:
                case B_BEETLE_1:
                case W_SPIDER_1:
                case B_SPIDER_1:
                    i++;
                    break;
                default:
                    break;
            }
        } else if (context->idToPos[i].z == 0)
            toCheck[checkSize++] = i;
    }

    int idx = 0;
    Hashmap_t visited;
    initHashmap(&visited, 512);
    for (uint_fast8_t i = 0; i < checkSize; ++i)
    {
        const int_fast8_t y = context->idToPos[toCheck[i]].y;
        const int_fast8_t x = context->idToPos[toCheck[i]].x;


        for (uint_fast8_t j = 0; j < 6; ++j)
        {
            const int_fast8_t newY1 = (int_fast8_t) (directions[j][0] + y);
            const int_fast8_t newX1 = (int_fast8_t) (directions[j][1] + x);

            if (context->board[MtA(0, newY1, newX1)] != NULLPIECE)
                continue;

            char key[7];
            sprintf(key, "%03d%03d", newY1, newX1);
            if (getByKey(key, &visited) != NULL)
                continue;

            int ignoreMe = 1;
            setByKey(key, &ignoreMe, sizeof(int), &visited);

            bool ok = true;
            for (int k = 0; k < 6; ++k)
            {
                const int_fast8_t newY2 = (int_fast8_t) (directions[k][0] + newY1);
                const int_fast8_t newX2 = (int_fast8_t) (directions[k][1] + newX1);

                const Pieces_t neighbor2 = context->board[MtA(0, newY2, newX2)];
                if (neighbor2 == NULLPIECE)
                    continue;
                if (neighbor2 < start || neighbor2 >= end)
                {
                    ok = false;
                    break;
                }
            }

            if (!ok)
                continue;

            for (uint8_t k = 0; k < addSize; ++k)
            {
                moves[idx++] = (Piece_t)
                {
                    toAdd[k],
                    {
                        0, newY1, newX1
                    }
                };
            }
        }
    }
    return NULL;
}

void queenMoves(const Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx)
{
    const int_fast8_t y = position->y;
    const int_fast8_t x = position->x;
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        if (board[MtA(0, newY, newX)] != NULLPIECE)
            continue;

        if (!canSlide(position, i, board))
            continue;


        const Piece_t move = {id, {0, newY, newX}};

        if (!hasNeighbor(&move, board))
            continue;

        moves[(*idx)++] = move;
    }
}
void* queen1Moves(void *arguments)
{
    const ThreadArgs_t *args = (ThreadArgs_t*) arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_QUEEN];

    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    queenMoves(id, &position, board, moves, &idx);
    return NULL;
}

void beetleMoves(const Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx)
{
    const int_fast8_t z = position->z;
    const int_fast8_t y = position->y;
    const int_fast8_t x = position->x;
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
            const Piece_t move = {id, {n, newY, newX}};
            moves[(*idx)++] = move;
            continue;
        }


        if (!canSlide(position, i, board))
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

        const Piece_t move = {id, {n, newY, newX}};
        if (!hasNeighbor(&move, board))
            continue;
        moves[(*idx)++] = move;
    }
}
void* beetle1Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_BEETLE_1];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_BEETLE_1 : B_BEETLE_1;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    beetleMoves(id, &position, board, moves, &idx);
    return NULL;
}
void* beetle2Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_BEETLE_2];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_BEETLE_2 : B_BEETLE_2;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    beetleMoves(id, &position, board, moves, &idx);
    return NULL;
}

void grasshopperMoves(const Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx)
{
    const int_fast8_t y = position->y;
    const int_fast8_t x = position->x;

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

        const Piece_t move = {id, {0, newY, newX}};
        moves[(*idx)++] = move;
    }
}
void *grasshopper1Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_GRASSHOPPER_1];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_GRASSHOPPER_1 : B_GRASSHOPPER_1;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    grasshopperMoves(id, &position, board, moves, &idx);
    return NULL;
}
void *grasshopper2Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_GRASSHOPPER_2];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_GRASSHOPPER_2 : B_GRASSHOPPER_2;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    grasshopperMoves(id, &position, board, moves, &idx);
    return NULL;
}
void *grasshopper3Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_GRASSHOPPER_3];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_GRASSHOPPER_3 : B_GRASSHOPPER_3;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    grasshopperMoves(id, &position, board, moves, &idx);
    return NULL;
}

void pillbugMoves(const Pieces_t id, const Position_t *position, const Context_t *context, Piece_t *moves, uint_fast8_t *idx)
{
    const Pieces_t last = context->lastMovedPiece;
    Position_t *positions = context->idToPos;
    const Pieces_t *board = context->board;

    const int_fast8_t y = position->y;
    const int_fast8_t x = position->x;

    int_fast8_t sizeFree = 0;
    Position_t freeLocations[6];

    Pieces_t startingPoint = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    bool visited[28] = {0};
    visited[id] = true;
    if (dfs(&positions[startingPoint], context, visited, true))
    {
        //Per muovere se stesso
        for (int_fast8_t i = 0; i < 6; ++i)
        {
            const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
            const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

            if (board[MtA(0, newY, newX)] != NULLPIECE)
                continue;

            const Position_t free = {0, newY, newX};
            freeLocations[sizeFree++] = free;
            if (!canSlide(position, i, board))
                continue;

            const Piece_t move = {id, free};
            if (!hasNeighbor(&move, board))
                continue;
            moves[(*idx)++] = move;
        }
    } else
    {
        for (int_fast8_t i = 0; i < 6; ++i)
        {
            const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
            const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

            if (board[MtA(0, newY, newX)] != NULLPIECE)
                continue;

            const Position_t free = {0, newY, newX};
            freeLocations[sizeFree++] = free;
        }
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
            memset(visited, 0, sizeof(visited));
            visited[neighbor] = true;
            if (!dfs(&positions[neighbor], context, visited, true))
            {
                visited[neighbor] = false;
                continue;
            }
            visited[neighbor] = false;
            moves[(*idx)++] = move;
        }
    }
}
void *pillbug1Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_PILLBUG];
    Pieces_t id = context->curColor == WHITE ? W_PILLBUG : B_PILLBUG;
    Position_t *positions = context->idToPos;
    Position_t position = positions[id];

    uint_fast8_t idx = 0;
    pillbugMoves(id, &position, context, moves, &idx);
    return NULL;
}

void ladybugMoves(const Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx)
{

    Hashmap_t hashmap;
    initHashmap(&hashmap, 512);
    const int_fast8_t y = position->y;
    const int_fast8_t x = position->x;
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

            if (newX2 == x && newY2 == y)
                continue;

            int_fast8_t n2;
            if (board[MtA(n1, newY2, newX2)] == NULLPIECE)
            {
                if (board[MtA(0, newY2, newX2)] == NULLPIECE)
                    continue;
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

                if (newX3 == newX2 && newY3 == newY2 && newX3 == x && newY3 == y)
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

                char key[7];
                sprintf(key, "%03d%03d", newY3, newX3);
                if (getByKey(key, &hashmap) != NULL)
                    continue;
                int ignoreMe = 1;
                setByKey(key, &ignoreMe, sizeof(int), &hashmap);

                moves[(*idx)++] = (Piece_t) {id, {0, newY3, newX3}};
            }
        }
    }
}
void *ladybug1Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_LADYBUG];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_LADYBUG : B_LADYBUG;
    Position_t *positions = context->idToPos;
    Position_t position = positions[id];

    uint_fast8_t idx = 0;
    ladybugMoves(id, &position, board, moves, &idx);
    return NULL;
}

void spiderMoves(const Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx)
{
    const int_fast8_t y = position->y;
    const int_fast8_t x = position->x;
    Hashmap_t hashmap;
    initHashmap(&hashmap, 512);

    //Primo passo
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY1 = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX1 = (int_fast8_t) (directions[i][1] + x);

        if (board[MtA(0, newY1, newX1)] != NULLPIECE)
            continue;

        if (!canSlide(position, i, board))
            continue;

        Piece_t piece1 = {id, {0, newY1, newX1}};
        if (!hasNeighbor(&piece1, board))
            continue;

        //Secondo passo
        for (int_fast8_t j = 0; j < 6; ++j)
        {
            const int_fast8_t newY2 = (int_fast8_t) (directions[j][0] + newY1);
            const int_fast8_t newX2 = (int_fast8_t) (directions[j][1] + newX1);

            if (newY2 == y && newX2 == x)
                continue;

            if (board[MtA(0, newY2, newX2)] != NULLPIECE)
                continue;

            const Position_t pos2 = {0, newY2, newX2};
            if (!canSlide(&pos2, j, board))
                continue;

            Piece_t piece2 = {id, {0, newY2, newX2}};
            if (!hasNeighbor(&piece2, board))
                continue;
            //Terzo passo
            for (int_fast8_t k = 0; k < 6; ++k)
            {
                const int_fast8_t newY3 = (int_fast8_t) (directions[k][0] + newY2);
                const int_fast8_t newX3 = (int_fast8_t) (directions[k][1] + newX2);

                if ((newY3 == y && newX3 == x) || (newY3 == newY1 && newX3 == newX1))
                    continue;

                if (board[MtA(0, newY3, newX3)] != NULLPIECE)
                    continue;

                const Position_t pos3 = {0, newY3, newX3};
                if (!canSlide(&pos3, j, board))
                    continue;


                const Piece_t move = {id, {0, newY3, newX3}};

                if (!hasNeighbor(&move, board))
                    continue;

                char key[7];
                sprintf(key, "%03d%03d", newY3, newX3);
                if (getByKey(key, &hashmap) != NULL)
                    continue;
                int ignoreMe = 1;
                setByKey(key, &ignoreMe, sizeof(int), &hashmap);

                moves[(*idx)++] = move;
            }
        }
    }
}
void *spider1Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_SPIDER_1];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_SPIDER_1 : B_SPIDER_1;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    spiderMoves(id, &position, board, moves, &idx);
    return NULL;
}
void *spider2Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_SPIDER_2];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_SPIDER_2 : B_SPIDER_2;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    spiderMoves(id, &position, board, moves, &idx);
    return NULL;
}

void antMoves(const Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx, Hashmap_t *visited)
{
    Hashmap_t hashmap;
    bool first = false;
    if (!visited)
    {
        first = true;
        initHashmap(&hashmap, 512);
        visited = &hashmap;
    }

    const int_fast8_t y = position->y;
    const int_fast8_t x = position->x;
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        if (board[MtA(0, newY, newX)] != NULLPIECE)
            continue;

        if (!canSlide(position, i, board))
            continue;

        char key[7];
        sprintf(key, "%03d%03d", newY, newX);
        if (getByKey(key, visited) != NULL)
            continue;

        int ignoreMe = 1;
        setByKey(key, &ignoreMe, sizeof(int), visited);

        const Piece_t move = {id, {0, newY, newX}};
        if (!hasNeighbor(&move, board))
            continue;
        moves[(*idx)++] = move;

        antMoves(id, &move.position, board, moves, idx, visited);
    }
    if (first)
        freeHashmap(visited);
}
void* ant1Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_ANT_1];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_ANT_1 : B_ANT_1;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    antMoves(id, &position, board, moves, &idx, NULL);
    return NULL;
}
void* ant2Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_ANT_2];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_ANT_2 : B_ANT_2;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    antMoves(id, &position, board, moves, &idx, NULL);
    return NULL;
}
void* ant3Moves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_ANT_3];
    Pieces_t *board = context->board;
    Pieces_t id = context->curColor == WHITE ? W_ANT_3 : B_ANT_3;
    Position_t position = context->idToPos[id];

    uint_fast8_t idx = 0;
    antMoves(id, &position, board, moves, &idx, NULL);
    return NULL;
}

void* mosquitoMoves(void *arguments)
{
    const ThreadArgs_t *args = arguments;
    const Context_t *context = args->context;
    Piece_t *moves = args->moves[B_MOSQUITO];
    const Pieces_t *board = context->board;
    const Pieces_t id = context->curColor == WHITE ? W_MOSQUITO : B_MOSQUITO;
    const Position_t *positions = context->idToPos;
    const Position_t position = positions[id];
    uint_fast8_t idx = 0;

    const int_fast8_t z = position.z;
    const int_fast8_t y = position.y;
    const int_fast8_t x = position.x;

    if (z > 0)
    {
        beetleMoves(id, &position, board, moves, &idx);
        return NULL;
    }

    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        const Pieces_t neighbor = board[MtA(0, newY, newX)];
        if (neighbor == NULLPIECE)
            continue;
        switch (neighbor)
        {
            case B_QUEEN:
            case W_QUEEN:
                queenMoves(id, &position, board, moves, &idx);
                break;
            case B_PILLBUG:
            case W_PILLBUG:
                pillbugMoves(id, &position, context, moves, &idx);
                break;
            case B_LADYBUG:
            case W_LADYBUG:
                ladybugMoves(id, &position, board, moves, &idx);
                break;
            case B_MOSQUITO:
            case W_MOSQUITO:
                break;
            case B_ANT_1:
            case B_ANT_2:
            case B_ANT_3:
            case W_ANT_1:
            case W_ANT_2:
            case W_ANT_3:
                antMoves(id, &position, board, moves, &idx, NULL);
                break;
            case B_GRASSHOPPER_1:
            case B_GRASSHOPPER_2:
            case B_GRASSHOPPER_3:
            case W_GRASSHOPPER_1:
            case W_GRASSHOPPER_2:
            case W_GRASSHOPPER_3:
                grasshopperMoves(id, &position, board, moves, &idx);
                break;
            case B_BEETLE_1:
            case B_BEETLE_2:
            case W_BEETLE_1:
            case W_BEETLE_2:
                beetleMoves(id, &position, board, moves, &idx);
                break;
            case B_SPIDER_1:
            case B_SPIDER_2:
            case W_SPIDER_1:
            case W_SPIDER_2:
                spiderMoves(id, &position, board, moves, &idx);
                break;
            default:
        }
    }
    return NULL;
}

void getMoves(const Context_t *context, Piece_t ***moves_ptr)
{
    const Pieces_t *board = context->board;
    const Position_t *positions = context->idToPos;
    const Colors_t color = context->curColor;
    const Pieces_t last = context->lastMovedPiece;

    Piece_t **moves = malloc(sizeof(Piece_t *) * MOVES_ARRAYS);
    moves_ptr[0] = moves;
    allocateMoves(moves);

    //Calcolo mosse nel turno 1 e 2 (hardcoded)
    if (context->turn == 1)
    {
        uint_fast8_t idx = 0;
        for (int i = W_PILLBUG; i < NUM_PIECES; ++i)
        {
            moves[14][idx++] = (Piece_t)
            {
                i,
                {
                    0, 0, 0
                }
            };
            switch (i)
            {
                case W_ANT_1:
                case W_GRASSHOPPER_1:
                    i += 2;
                    break;
                case W_ANT_2:
                case W_GRASSHOPPER_2:
                case W_BEETLE_1:
                case W_SPIDER_1:
                    i++;
                    break;
                default:
                    break;
            }
        }
        return;
    }
    if (context->turn == 2)
    {
        uint_fast8_t idx = 0;
        for (int i = B_QUEEN; i < W_QUEEN; ++i)
        {
            for (uint_fast8_t j = 0; j < 6; ++j)
            {
                moves[14][idx++] = (Piece_t)
                {
                    i,
                    {
                        0, directions[j][0], directions[j][1]
                    }
                };
            }
            switch (i)
            {
                case B_ANT_1:
                case B_GRASSHOPPER_1:
                    i += 2;
                    break;
                case B_ANT_2:
                case B_GRASSHOPPER_2:
                case B_BEETLE_1:
                case B_SPIDER_1:
                    i++;
                    break;
                default:
                    break;
            }
        }
        return;
    }

    const Pieces_t start = color == WHITE ? 14 : 0;
    const Pieces_t end = start + 14;

    //Force queen
    if ((context->turn == 7 && positions[W_QUEEN].z == -1) || (context->turn == 8 && positions[B_QUEEN].z == -1))
    {
        int idx = 0;
        Hashmap_t visited;
        initHashmap(&visited, 512);
        for (uint_fast8_t i = start + 1; i < end; ++i)
        {
            const int_fast8_t z = positions[i].z;
            if (z != 0)
                continue;
            const int_fast8_t y = positions[i].y;
            const int_fast8_t x = positions[i].x;


            for (uint_fast8_t j = 0; j < 6; ++j)
            {
                const int_fast8_t newY1 = (int_fast8_t) (directions[j][0] + y);
                const int_fast8_t newX1 = (int_fast8_t) (directions[j][1] + x);

                if (board[MtA(0, newY1, newX1)] != NULLPIECE)
                    continue;

                char key[7];
                sprintf(key, "%03d%03d", newY1, newX1);
                if (getByKey(key, &visited) != NULL)
                    continue;

                int ignoreMe = 1;
                setByKey(key, &ignoreMe, sizeof(int), &visited);

                bool ok = true;
                for (int k = 0; k < 6; ++k)
                {
                    const int_fast8_t newY2 = (int_fast8_t) (directions[k][0] + newY1);
                    const int_fast8_t newX2 = (int_fast8_t) (directions[k][1] + newX1);

                    const Pieces_t neighbor2 = context->board[MtA(0, newY2, newX2)];
                    if (neighbor2 == NULLPIECE)
                        continue;
                    if (neighbor2 < start || neighbor2 >= end)
                    {
                        ok = false;
                        break;
                    }
                }

                if (!ok)
                    continue;

                moves[14][idx++] = (Piece_t)
                {
                    color == WHITE ? W_QUEEN : B_QUEEN,
                    {
                        0, newY1, newX1
                    }
                };
            }
        }
        return;
    }


    pthread_t threads[MOVES_ARRAYS];
    bool created[MOVES_ARRAYS] = {false};

    //add
    ThreadArgs_t args = {context, moves};
    pthread_create(&threads[14], NULL, addMoves, &args);
    created[14] = true;

    void* (*funcs[])(void *) = {queen1Moves, pillbug1Moves, ladybug1Moves, mosquitoMoves, ant1Moves, ant2Moves, ant3Moves, grasshopper1Moves, grasshopper2Moves, grasshopper3Moves, beetle1Moves, beetle2Moves, spider1Moves, spider2Moves};
    //se la regina non c'è i pezzi non possono muoversi
    if (positions[color == WHITE ? W_QUEEN : B_QUEEN].z != -1)
    {
        for (uint_fast8_t i = 0; i < 14; ++i)
        {
            if (positions[i].z == -1)
                continue;
            bool visited[28];
            visited[color == WHITE ? i + 14 : i] = true;
            const Pieces_t startingPoint = chooseStartingPoint(i, color, positions);
            if (i != B_PILLBUG && !dfs(&positions[startingPoint], context, visited, true))
                continue;
            pthread_create(&threads[i], NULL, funcs[i], &args);
            created[i] = true;
        }

    }

    //join
    for (int i = 0; i < MOVES_ARRAYS; ++i)
    {
        if (created[i])
        //TODO: per il futuro, se vogliamo iniziare a creare i nodi dei thread che finiscono prima, tocca studia un altro metodo
            pthread_join(threads[i], NULL);
    }

}

