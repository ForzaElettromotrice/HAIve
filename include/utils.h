//
// Created by f3m on 02/04/25.
//

#pragma once

#include <stdlib.h>
#include <string.h>
#include <xxhash.h>
#include <stdio.h>

#include "enums.h"


#define BOARD_SIZE 5 * 56 * 28

#define MtA(z,y,x) (z) * 56 * 28 + (y + 28) * 28 + (x + 14)


uint64_t hashAll(const Pieces_t *board, const Position_t *positions);
int convertFromMZinga(char* mzinga_string, Context_t* context);
void debugPrint(Context_t* context);
void playMove(Context_t* context, char* move);
int parseMove(Context_t* context, char* move);
void addMove(Context_t* context, char* move);
