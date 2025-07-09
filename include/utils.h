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


#define BOARD_SIZE (7 * 28 * 56)
#define BOARD_Y 28
#define BOARD_X 56

#define MtA(z,y,x) ((z) * 56 * 28 + ((y) + 14) * 56 + ((x) + 28))
#define yOf(y) (y + 14)
#define xOf(x) (x + 28)

#define isBlack(x) (x < 14)
#define isWhite(x) (x >= 14)

extern const Piece_t pass;

uint64_t hashAll(const Pieces_t *board, const Position_t *positions, Colors_t color);

void initContext(Context_t *context);
void resetContext(Context_t *context);
void copyContext(const Context_t *src, Context_t *dst);
void cleanContext(const Context_t *context);

Command_t parseCommand(const char *command);

Piece_t parseMove(const Position_t *idToPos, char *move);
void doMove(Context_t *context, char *move);
void printMove(const Context_t *context, const Piece_t *move);

void printInfo();
void printGameString(const Context_t *context);

GameStatus_t getGameStatus(const Context_t *context);
int_fast8_t howManyAround(const Context_t *context, Pieces_t id, bool friendly);
void addOurMove(Context_t *context, const Piece_t *move);

bool isContextEnded(const Context_t *context);
void printPos(const Position_t *idToPos);

#ifdef __cplusplus
}
#endif
