//
// Created by f3m on 19/05/25.
//

#pragma once

#include <enums.h>

#define BOARD_SIZE 7 * 28 * 56
#define MtA(z,y,x) (z) * 56 * 28 + (y + 14) * 56 + (x + 28)

int initContext(Context_t *context);
void cleanContext(const Context_t *context);


void printInfo();
void printGameString(const Context_t *context);
#define isBlack(x) (x < 14)
#define isWhite(x) (x >= 14)

void addMazingaMove(const char *move, Context_t *context);

int parseGameTypeString(const char *parameters, Context_t *context);
int parseGameString(char *parameters, Context_t *context);
Pieces_t parsePiece(const char *piece, Colors_t color);
void contextNewGameMLP(Context_t *context);