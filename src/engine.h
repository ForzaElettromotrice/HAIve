//
// Created by minga on 16/01/2025.
//

#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "enums.h"
#include "logger.h"

typedef struct Piece
{
    Pieces_t type;
    Colors_t color;
    int8_t x;
    int8_t y;
    int8_t z;
} Piece_t;


int initGame();
void cleanGame();

bool isEncodingValid(const char *encoding, int8_t *move, bool *add);
bool isMoveValid(const int8_t *move, uint8_t *idx, bool add);

void movePiece(uint8_t idx, int8_t x, int8_t y, int8_t z);

void printBoardStatus();
