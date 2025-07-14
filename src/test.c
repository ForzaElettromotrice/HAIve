//
// Created by f3m on 11/07/25.
//

#include <test.h>
#include <logger.h>
#include <string.h>

#include "utils.h"

void convertAndSet(const char *tok1, const char *tok2, const char *tok3, const Pieces_t p, Pieces_t *board, Position_t *idToPos)
{
    if (idToPos == NULL || board == NULL)
    {
        logE(stderr, "Non dovrebbe mai succedere\n");
        return;
    }

    const int8_t z = (int8_t) strtol(tok1, NULL, 10);
    const int8_t y = (int8_t) strtol(tok2, NULL, 10);
    const int8_t x = (int8_t) strtol(tok3, NULL, 10);

    logD(stdout, "%d %d %d %d\n", z, y, x, p);

    idToPos[p] = (Position_t){z, y, x};
    board[MtA(z, y, x)] = p;
}
void parseLine(const char *const line, Context_t *context, const bool white)
{
    context->board = malloc(BOARD_SIZE * sizeof(Pieces_t));
    context->moves = calloc(1024, sizeof(char));
    context->idToPos = malloc(NUM_PIECES * sizeof(Position_t));
    context->movesSize = 1024;
    memset(context->board, 0xff, BOARD_SIZE * sizeof(Pieces_t));
    memset(context->idToPos, 0xff, NUM_PIECES * sizeof(Position_t));
    memset(context->moves, 0x00, context->movesSize);

    context->turn = white ? 5 : 6;
    context->curColor = white ? WHITE : BLACK;
    context->gameStatus = IN_PROGRESS;
    context->lastMovedPiece = NULLPIECE;

    char *lline = strdup(line);
    logD(stdout, "Line: %s", line);
    logD(stdout, "Line: %s", lline);
    const char *tok1 = strtok(lline, ",");
    const char *tok2 = strtok(NULL, ",");
    const char *tok3 = strtok(NULL, ",");
    convertAndSet(tok1, tok2, tok3, B_QUEEN, context->board, context->idToPos);

    for (int i = 1; i < 28; ++i)
    {
        tok1 = strtok(NULL, ",");
        tok2 = strtok(NULL, ",");
        tok3 = strtok(NULL, ",");
        logD(stdout, "%s %s %s\n", tok1, tok2, tok3);
        // convertAndSet(tok1, tok2, tok3, i, context->board, context->idToPos);
    }

    free(lline);
}

void loadContexts(Context_t **contexts)
{
    FILE *file = fopen(TEST_PATH, "r");

    char *line = NULL;
    size_t len = 0;
    int lines = 0;

    while (getline(&line, &len, file) != -1)
        lines++;

    logD(stdout, "Total lines: %d\n", lines);
    *contexts = malloc(lines * 2 * sizeof(Context_t));

    fseek(file, 0, SEEK_SET);

    uint16_t idx = 0;
    while (getline(&line, &len, file) != -1)
    {
        logD(stdout, "Idx: %d\n", idx);
        logD(stdout, "Line: %s", line);
        parseLine(line, &(*contexts)[idx++], true);
        logD(stdout, "Line: %s", line);
        idx++;
        // parseLine(line, &(*contexts)[idx++], false);
        if (idx > 32)
            break;
    }


    fclose(file);
}

void testMoves()
{
    struct timespec start[15];
    struct timespec end[15];
    Context_t *contexts;
    loadContexts(&contexts);

    // clock_gettime(CLOCK_MONOTONIC, &start);
}
