//
// Created by f3m on 11/07/25.
//

#include <float.h>
#include <test.h>
#include <logger.h>
#include <string.h>
#include <time.h>

#include "moves.h"
#include "utils.h"

typedef struct agaga
{
    const Context_t *context;
    Piece_t *moves;
} agaga_t;

typedef struct delta
{
    double delta;
    int64_t count;
} delta_t;

delta_t delta[15] = {};

void initContextTest(Context_t *context, const bool white)
{
    context->board = malloc(BOARD_SIZE * sizeof(Pieces_t));
    context->idToPos = malloc(NUM_PIECES * sizeof(Position_t));
    memset(context->board, 0xff, BOARD_SIZE * sizeof(Pieces_t));
    memset(context->idToPos, 0xff, NUM_PIECES * sizeof(Position_t));

    context->turn = white ? 5 : 6;
    context->curColor = white ? WHITE : BLACK;
    context->gameStatus = IN_PROGRESS;
    context->lastMovedPiece = NULLPIECE;
}
void freeContextTest(const Context_t *context)
{
    free(context->board);
    free(context->idToPos);
}
void updatePiece(const Pieces_t piece, const char *tok1, const char *tok2, const char *tok3, const Context_t *context)
{
    const Position_t pos = {(int8_t) strtol(tok1, NULL, 10), (int8_t) strtol(tok2, NULL, 10), (int8_t) strtol(tok3, NULL, 10)};
    if (pos.z == -1)
        return;
    context->idToPos[piece] = pos;
    context->board[MtA(pos.z, pos.y, pos.x)] = piece;
}
void parseLine(const char *line, const Context_t *context)
{
    char *lline = strdup(line);
    const char *tok1 = strtok(lline, ",");
    const char *tok2 = strtok(NULL, ",");
    const char *tok3 = strtok(NULL, ",");

    updatePiece(B_QUEEN, tok1, tok2, tok3, context);

    for (int i = 1; i < 28; ++i)
    {
        tok1 = strtok(NULL, ",");
        tok2 = strtok(NULL, ",");
        tok3 = strtok(NULL, ",");
        updatePiece(i, tok1, tok2, tok3, context);
    }
    free(lline);
}
void loadContexts(const Context_t *contexts)
{
    FILE *file = fopen(TEST_PATH, "r");

    size_t size;
    char *line = NULL;
    size_t read;
    uint16_t idx = 0;
    while ((read = getline(&line, &size, file)) != -1)
    {
        line[read - 1] = '\0';
        parseLine(line, &contexts[idx++]);
        parseLine(line, &contexts[idx++]);
    }

    free(line);
    fclose(file);
}


void test(const Context_t *context)
{
    struct timespec start[15];
    struct timespec end[15];

    memset(start, 0, sizeof(start));
    memset(end, 0, sizeof(end));

    Piece_t *moves = malloc(3000 * sizeof(Piece_t));
    uint_fast8_t idx = 0;

    Pieces_t piece = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    const Position_t *pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[B_QUEEN] = true;
        visited[W_QUEEN] = true;
        if (dfs(&context->idToPos[context->curColor == WHITE ? B_QUEEN : W_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_QUEEN]);
            queenMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_QUEEN]);
            delta[B_QUEEN].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_PILLBUG : B_PILLBUG;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        clock_gettime(CLOCK_MONOTONIC, &start[B_PILLBUG]);
        pillbugMoves(piece, pos, context, moves, &idx);
        clock_gettime(CLOCK_MONOTONIC, &end[B_PILLBUG]);
        delta[B_PILLBUG].count++;
    }

    idx = 0;
    piece = context->curColor == WHITE ? W_LADYBUG : B_LADYBUG;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_LADYBUG]);
            ladybugMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_LADYBUG]);
            delta[B_LADYBUG].count++;
        }
    }

    agaga_t arg = {context, moves};
    piece = context->curColor == WHITE ? W_MOSQUITO : B_MOSQUITO;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_MOSQUITO]);
            mosquitoMoves(&arg);
            clock_gettime(CLOCK_MONOTONIC, &end[B_MOSQUITO]);
            delta[B_MOSQUITO].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_ANT_1 : B_ANT_1;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_ANT_1]);
            antMoves(piece, pos, context->board, moves, &idx, NULL);
            clock_gettime(CLOCK_MONOTONIC, &end[B_ANT_1]);
            delta[B_ANT_1].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_ANT_2 : B_ANT_2;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_ANT_2]);
            antMoves(piece, pos, context->board, moves, &idx, NULL);
            clock_gettime(CLOCK_MONOTONIC, &end[B_ANT_2]);
            delta[B_ANT_2].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_ANT_3 : B_ANT_3;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_ANT_3]);
            antMoves(piece, pos, context->board, moves, &idx, NULL);
            clock_gettime(CLOCK_MONOTONIC, &end[B_ANT_3]);
            delta[B_ANT_3].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_GRASSHOPPER_1 : B_GRASSHOPPER_1;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_GRASSHOPPER_1]);
            grasshopperMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_GRASSHOPPER_1]);
            delta[B_GRASSHOPPER_1].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_GRASSHOPPER_2 : B_GRASSHOPPER_2;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_GRASSHOPPER_2]);
            grasshopperMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_GRASSHOPPER_2]);
            delta[B_GRASSHOPPER_2].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_GRASSHOPPER_3 : B_GRASSHOPPER_3;
    pos = &context->idToPos[piece];

    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_GRASSHOPPER_3]);
            grasshopperMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_GRASSHOPPER_3]);
            delta[B_GRASSHOPPER_3].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_BEETLE_1 : B_BEETLE_1;
    pos = &context->idToPos[piece];
    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_BEETLE_1]);
            beetleMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_BEETLE_1]);
            delta[B_BEETLE_1].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_BEETLE_2 : B_BEETLE_2;
    pos = &context->idToPos[piece];
    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_BEETLE_2]);
            beetleMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_BEETLE_2]);
            delta[B_BEETLE_2].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_SPIDER_1 : B_SPIDER_1;
    pos = &context->idToPos[piece];
    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_SPIDER_1]);
            spiderMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_SPIDER_1]);
            delta[B_SPIDER_1].count++;
        }
    }
    idx = 0;
    piece = context->curColor == WHITE ? W_SPIDER_2 : B_SPIDER_2;
    pos = &context->idToPos[piece];
    if (pos->z != -1)
    {
        bool visited[28] = {};
        visited[piece] = true;
        visited[B_QUEEN] = true;
        if (dfs(&context->idToPos[B_QUEEN], context, visited, true))
        {
            clock_gettime(CLOCK_MONOTONIC, &start[B_SPIDER_2]);
            spiderMoves(piece, pos, context->board, moves, &idx);
            clock_gettime(CLOCK_MONOTONIC, &end[B_SPIDER_2]);
            delta[B_SPIDER_2].count++;
        }
    }
    agaga_t args = {context, moves};

    clock_gettime(CLOCK_MONOTONIC, &start[14]);
    addMoves(&args);
    clock_gettime(CLOCK_MONOTONIC, &end[14]);
    delta[14].count++;

    for (int i = 0; i < 15; ++i)
    {
        delta[i].delta += (double) (end[i].tv_sec - start[i].tv_sec) * 1e6
                + (double) (end[i].tv_nsec - start[i].tv_nsec) / 1e3;
    }

    free(moves);
}


void testMoves()
{
    Context_t *contexts = malloc(NUM_CONTEXTS * sizeof(Context_t));

    for (int i = 0; i < NUM_CONTEXTS; ++i)
        initContextTest(&contexts[i], i % 2);

    loadContexts(contexts);

    for (int i = 0; i < NUM_CONTEXTS; ++i)
    {
        test(&contexts[i]);
    }

    for (int i = 0; i < 15; ++i)
    {
        delta[i].delta /= (double) delta[i].count;
        logD(stdout, "Average time for %d: %f microseconds\n", i, delta[i].delta);
        // logD(stdout, "Delta %d %f %ld\n", i, delta[i].delta, delta[i].count);
    }

    for (int i = 0; i < NUM_CONTEXTS; ++i)
        freeContextTest(&contexts[i]);
    free(contexts);
}
