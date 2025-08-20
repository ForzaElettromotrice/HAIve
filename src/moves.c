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
    const HAIveContext_t *context;
    Piece_t *moves;
} ThreadArgs_t;

void alertMoves(const Pieces_t piece, const uint16_t idx)
{
    if (idx > 160)
    {
        logE(stderr, "Piece %d idx > 140! idx = %d\n", piece, idx);
        return;
    }
    if (idx > 140)
    {
        logE(stderr, "Piece %d idx > 140! idx = %d\n", piece, idx);
        return;
    }
    if (idx > 120)
    {
        logE(stderr, "Piece %d idx > 120!\n", piece);
        return;
    }
}
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
bool dfs(const Position_t *start, const HAIveContext_t *context, bool *visited, const bool first)
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
    }

    if (first)
    {
        for (int_fast8_t i = 0; i < 28; ++i)
            if (!visited[i] && positions[i].z == 0)
                return false;
    }
    return true;
}
bool canMove(const Pieces_t id, const HAIveContext_t *context)
{
    if (context->idToPos[id].z == -1)
        return false;
    bool visited[28] = {};
    visited[id] = true;
    const Pieces_t startingPoint = chooseStartingPoint(id, context->curColor, context->idToPos);
    visited[startingPoint] = true;
    if (!dfs(&context->idToPos[startingPoint], context, visited, true))
        return false;
    return true;
}


void *addMoves(const HAIveContext_t *context, Piece_t *moves, uint_fast8_t *idx)
{
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
            if (getByStr(key, &visited) != NULL)
                continue;

            int ignoreMe = 1;
            setByStr(key, &ignoreMe, sizeof(int), &visited);

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
                alertMoves(NULLPIECE, *idx);
                moves[(*idx)++] = (Piece_t)
                {
                    toAdd[k],
                    {
                        0, newY1, newX1
                    }
                };
            }
        }
    }
    freeHashmap(&visited);
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

        alertMoves(id, *idx);
        moves[(*idx)++] = move;
    }
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

        alertMoves(id, *idx);
        moves[(*idx)++] = move;
    }
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

        alertMoves(id, *idx);
        moves[(*idx)++] = move;
    }
}
void pillbugMoves(const Pieces_t id, const Position_t *position, const HAIveContext_t *context, Piece_t *moves, uint_fast8_t *idx)
{
    const Pieces_t last = context->lastMovedPiece;
    const Position_t *positions = context->idToPos;
    const Pieces_t *board = context->board;

    const int_fast8_t y = position->y;
    const int_fast8_t x = position->x;

    int_fast8_t sizeFree = 0;
    Position_t freeLocations[6];

    const Pieces_t startingPoint = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    bool visited[28] = {0};
    visited[id] = true;
    visited[startingPoint] = true;
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

            alertMoves(id, *idx);
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
            visited[chooseStartingPoint(neighbor, context->curColor, positions)] = true;
            if (!dfs(&positions[chooseStartingPoint(neighbor, context->curColor, positions)], context, visited, true))
            {
                visited[neighbor] = false;
                continue;
            }
            visited[neighbor] = false;

            alertMoves(id, *idx);
            moves[(*idx)++] = move;
        }
    }
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

                // Don't step back to previous or original tile
                if ((newY3 == newY2 && newX3 == newX2) || (newY3 == y && newX3 == x))
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
                if (getByStr(key, &hashmap) != NULL)
                    continue;
                int ignoreMe = 1;
                setByStr(key, &ignoreMe, sizeof(int), &hashmap);

                alertMoves(id, *idx);
                moves[(*idx)++] = (Piece_t){id, {0, newY3, newX3}};
            }
        }
    }
    freeHashmap(&hashmap);
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
                if (getByStr(key, &hashmap) != NULL)
                    continue;
                int ignoreMe = 1;
                setByStr(key, &ignoreMe, sizeof(int), &hashmap);

                alertMoves(id, *idx);
                moves[(*idx)++] = move;
            }
        }
    }
    freeHashmap(&hashmap);
}
void antMoves(const Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx, Hashmap_t *visited)
{
    Hashmap_t hashmap = {};
    bool first = false;
    if (!visited)
    {
        first = true;
        initHashmap(&hashmap, 4096);
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
        if (getByStr(key, visited) != NULL)
            continue;

        const Piece_t move = {id, {0, newY, newX}};
        if (!hasNeighbor(&move, board))
            continue;

        int ignoreMe = 1;
        setByStr(key, &ignoreMe, sizeof(int), visited);

        alertMoves(id, *idx);
        if (*idx == 255)
        {
            logE(stderr, "Too many moves for ant %d!\n", id);
        }
        moves[(*idx)++] = move;

        antMoves(id, &move.position, board, moves, idx, visited);
    }
    if (first)
        freeHashmap(&hashmap);
}
void mosquitoMoves(const Pieces_t id, const HAIveContext_t *context, Piece_t *moves, uint_fast8_t *idx)
{
    const Position_t position = context->idToPos[id];

    const int_fast8_t z = position.z;
    const int_fast8_t y = position.y;
    const int_fast8_t x = position.x;

    if (z > 0)
    {
        beetleMoves(id, &position, context->board, moves, idx);
        return;
    }

    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        const Pieces_t neighbor = context->board[MtA(0, newY, newX)];
        switch (neighbor)
        {
            case B_QUEEN:
            case W_QUEEN:
                queenMoves(id, &position, context->board, moves, idx);
                break;
            case B_PILLBUG:
            case W_PILLBUG:
                pillbugMoves(id, &position, context, moves, idx);
                break;
            case B_LADYBUG:
            case W_LADYBUG:
                ladybugMoves(id, &position, context->board, moves, idx);
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
                antMoves(id, &position, context->board, moves, idx, NULL);
                break;
            case B_GRASSHOPPER_1:
            case B_GRASSHOPPER_2:
            case B_GRASSHOPPER_3:
            case W_GRASSHOPPER_1:
            case W_GRASSHOPPER_2:
            case W_GRASSHOPPER_3:
                grasshopperMoves(id, &position, context->board, moves, idx);
                break;
            case B_BEETLE_1:
            case B_BEETLE_2:
            case W_BEETLE_1:
            case W_BEETLE_2:
                beetleMoves(id, &position, context->board, moves, idx);
                break;
            case B_SPIDER_1:
            case B_SPIDER_2:
            case W_SPIDER_1:
            case W_SPIDER_2:
                spiderMoves(id, &position, context->board, moves, idx);
                break;
            case NULLPIECE:
                break;
        }
    }
}


void getMoves(const HAIveContext_t *context, Piece_t *moves)
{
    const Pieces_t *board = context->board;
    const Position_t *positions = context->idToPos;
    const Colors_t color = context->curColor;

    memset(moves, 0xff, MOVES_SIZE * sizeof(Piece_t));


    //Calcolo mosse nel turno 1 e 2 (hardcoded)
    if (context->turn == 1)
    {
        uint_fast8_t idx = 0;
        for (int i = W_PILLBUG; i < NUM_PIECES; ++i)
        {
            moves[idx++] = (Piece_t){i, {0, 0, 0}};
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
        for (int i = B_QUEEN + 1; i < W_QUEEN; ++i)
        {
            for (uint_fast8_t j = 0; j < 6; ++j)
            {
                moves[idx++] = (Piece_t){i, {0, directions[j][0], directions[j][1]}};
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
                if (getByStr(key, &visited) != NULL)
                    continue;

                int ignoreMe = 1;
                setByStr(key, &ignoreMe, sizeof(int), &visited);

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

                moves[idx++] = (Piece_t){color == WHITE ? W_QUEEN : B_QUEEN, {0, newY1, newX1}};
            }
        }
        return;
    }

    uint_fast8_t idx = 0;

    addMoves(context, moves, &idx);
    //Se la queen non c'è non si può muovere nulla
    if (positions[color == WHITE ? W_QUEEN : B_QUEEN].z == -1)
        return;

    const int8_t addId = color == WHITE ? 14 : 0;
    if (canMove(B_QUEEN + addId, context))
        queenMoves(B_QUEEN + addId, &positions[B_QUEEN + addId], board, moves, &idx);

    if (canMove(B_PILLBUG + addId, context))
        pillbugMoves(B_PILLBUG + addId, &positions[B_PILLBUG + addId], context, moves, &idx);

    if (canMove(B_LADYBUG + addId, context))
        ladybugMoves(B_LADYBUG + addId, &positions[B_LADYBUG + addId], board, moves, &idx);

    if (canMove(B_MOSQUITO + addId, context))
        mosquitoMoves(B_MOSQUITO + addId, context, moves, &idx);

    if (canMove(B_ANT_1 + addId, context))
        antMoves(B_ANT_1 + addId, &positions[B_ANT_1 + addId], board, moves, &idx, NULL);

    if (canMove(B_ANT_2 + addId, context))
        antMoves(B_ANT_2 + addId, &positions[B_ANT_2 + addId], board, moves, &idx, NULL);

    if (canMove(B_ANT_3 + addId, context))
        antMoves(B_ANT_3 + addId, &positions[B_ANT_3 + addId], board, moves, &idx, NULL);

    if (canMove(B_GRASSHOPPER_1 + addId, context))
        grasshopperMoves(B_GRASSHOPPER_1 + addId, &positions[B_GRASSHOPPER_1 + addId], board, moves, &idx);

    if (canMove(B_GRASSHOPPER_2 + addId, context))
        grasshopperMoves(B_GRASSHOPPER_2 + addId, &positions[B_GRASSHOPPER_2 + addId], board, moves, &idx);

    if (canMove(B_GRASSHOPPER_3 + addId, context))
        grasshopperMoves(B_GRASSHOPPER_3 + addId, &positions[B_GRASSHOPPER_3 + addId], board, moves, &idx);

    if (canMove(B_BEETLE_1 + addId, context))
        beetleMoves(B_BEETLE_1 + addId, &positions[B_BEETLE_1 + addId], board, moves, &idx);

    if (canMove(B_BEETLE_2 + addId, context))
        beetleMoves(B_BEETLE_2 + addId, &positions[B_BEETLE_2 + addId], board, moves, &idx);

    if (canMove(B_SPIDER_1 + addId, context))
        spiderMoves(B_SPIDER_1 + addId, &positions[B_SPIDER_1 + addId], board, moves, &idx);

    if (canMove(B_SPIDER_2 + addId, context))
        spiderMoves(B_SPIDER_2 + addId, &positions[B_SPIDER_2 + addId], board, moves, &idx);
}

