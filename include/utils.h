//
// Created by f3m on 28/05/25.
//

#pragma once

#include <enums.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#define BOARD_SIZE (7 * 56 * 112)
#define BOARD_Y 56
#define BOARD_X 112

#define MtA(z,y,x) ((z) * 56 * 112 + ((y) + 28) * 112 + ((x) + 56))
#define yOf(y) (y + 28)
#define xOf(x) (x + 56)

#define isBlack(x) (x < 14)
#define isWhite(x) (x >= 14)

extern const Piece_t pass;

uint64_t hashAll(const Pieces_t *board, const Position_t *positions, Colors_t color);

void initMzingaContext(MzingaContext_t *context);
void resetMzingaContext(MzingaContext_t *context);
void cleanMzingaContext(const MzingaContext_t *context);

void initHAIveContext(HAIveContext_t *context);
void resetHAIveContext(HAIveContext_t *context);
void copyHAIveContext(const HAIveContext_t *src, HAIveContext_t *dst);
void cleanHAIveContext(const HAIveContext_t *context);

Command_t parseCommand(const char *command);

Piece_t parseMove(const Position_t *idToPos, char *move);
void printMove(const HAIveContext_t *context, const Piece_t *move);

void printInfo();
void printGameString(const MzingaContext_t *context);

GameStatus_t getGameStatus(const HAIveContext_t *context);
int_fast8_t howManyAround(const HAIveContext_t *context, Pieces_t id, bool friendly);
void addMazingaMove(MzingaContext_t *context, const char *move);
void addHAIveMove(HAIveContext_t *context, const Piece_t *move);

bool isContextEnded(const HAIveContext_t *context);


#ifdef __cplusplus
}
#endif
