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
void debugPrint(Context_t* context); // Prints the context - for debug purposes
void playMove(Context_t* context, char* move); // From a char* move, plays it
int parseMove(Context_t* context, char* move); // Parses the move, and sets it in the context
void addMove(Context_t* context, char* move); // Append the char* move to the context->moves
int convertFromMZinga(char* mzinga_string, Context_t* context); // Sets the context FROM A WHOLE MZINGA STRING
char* deconvertMove(Context_t* context, Pieces_t pieceMoved);
