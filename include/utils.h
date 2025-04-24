//
// Created by f3m on 02/04/25.
//

#pragma once

#include <stdlib.h>
#include <string.h>
#include <xxhash.h>
#include <stdio.h>

#include "enums.h"


#define BOARD_SIZE 5 * 28 * 56

#define MtA(z,y,x) (z) * 56 * 28 + (y + 14) * 56 + (x + 28)


uint64_t hashAll(const Pieces_t *board, const Position_t *positions);
int convertFromMZinga(char* mzinga_string, Context_t* context);
void debugPrint(Context_t* context);
void playMove(Context_t* context, char* move);
void parseMove(Context_t* context, char* move);
void addMove(Context_t* context, char* move);
