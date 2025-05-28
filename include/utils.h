//
// Created by f3m on 28/05/25.
//

#pragma once

#include <enums.h>

#define BOARD_SIZE 7 * 28 * 56
#define MtA(z,y,x) (z) * 56 * 28 + (y + 14) * 56 + (x + 28)


void initContext(Context_t *context);
void resetContext(Context_t *context);
void copyContext(const Context_t *src, Context_t *dst);
void cleanContext(const Context_t *context);


Command_t parseCommand(const char *command);


void doMove(Context_t *context, char *move);
void bestMove(const Context_t *context);


void printInfo();
void printGameString(const Context_t *context);
