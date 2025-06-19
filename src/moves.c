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

typedef struct ThreadArgs
{
    const Context_t *context;
    Piece_t *moves;
} ThreadArgs_t;

void allocateMoves(Piece_t **moves)
{
    //TODO: fare una versione che alloca il giusto per ogni pezzo e vedere quale va piu veloce
    //Allocazione delle mosse
    for (int i = 0; i < 14; ++i)
    {
        //FIXME: 70 è provvisorio, un valore più preciso potrebbe fare più comodo
        moves[i] = malloc(70 * sizeof(Position_t));
        memset(moves[i], 0xff, 70 * sizeof(Position_t));
    }
}
void addMoves(const ThreadArgs_t *args)
{
    const Context_t *context = args->context;
    Piece_t *moves = args->moves;

    const Pieces_t start = context->curColor == WHITE ? 14 : 0;
    const Pieces_t end = start + 14;


    uint8_t addSize = 0;
    uint8_t checkSize = 0;
    int8_t toAdd[14];
    int8_t toCheck[14];
    for (int_fast8_t i = start; i < end; ++i)
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
        const int_fast8_t y = context->idToPos[i].y;
        const int_fast8_t x = context->idToPos[i].x;


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
                moves[idx++] = (Piece_t){toAdd[k], {0, newY1, newX1}};
            }
        }
    }
}
void getMoves(const Context_t *context, Piece_t **moves)
{
    const Pieces_t *board = context->board;
    const Position_t *positions = context->idToPos;
    const Colors_t color = context->curColor;
    const Pieces_t last = context->lastMovedPiece;

    allocateMoves(moves);


    //Calcolo mosse nel turno 1 e 2 (hardcoded)
    if (context->turn == 1)
    {
        uint_fast8_t idx = 0;
        for (int i = W_PILLBUG; i < NUM_PIECES; ++i)
        {
            moves[14][idx++] = (Piece_t){i, {0, 0, 0}};
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
                moves[14][idx++] = (Piece_t){i, {0, directions[j][0], directions[j][1]}};
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

                moves[14][idx++] = (Piece_t){W_QUEEN, {0, newY1, newX1}};
            }
        }
        return;
    }


    pthread_t threads[MOVES_ARRAYS];

    //add
    ThreadArgs_t addTh = {context, moves[14]};
    pthread_create(&threads[14], NULL, addMoves, &addTh);

    //se la regina non c'è i pezzi non possono muoversi
    if (positions[color == WHITE ? W_QUEEN : B_QUEEN].z != -1)
    {
    }

    //join
    for (int i = 0; i < MOVES_ARRAYS; ++i)
    {
        //TODO: per il futuro, se vogliamo iniziare a creare i nodi dei thread che finiscono prima, tocca studia un altro metodo
        pthread_join(threads[i], NULL);
    }
}

uint16_t getMovesSize(const Context_t *context, const Piece_t *moves) {

    uint16_t mSize = 0;
    while (moves[mSize].id != NULLPIECE)
        mSize++;
    return mSize;

}