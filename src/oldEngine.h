//
// Created by minga on 16/01/2025.
//

#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include "enums.h"
#include "logger.h"

typedef struct Piece
{
    Pieces_t id;
    int8_t x;
    int8_t y;
    int8_t z;
} Piece_t;


int initGame();
void cleanGame();

bool isEncodingValid(const char *encoding, int8_t *move);
bool isMoveValid(uint8_t id, int8_t x, int8_t y, int8_t z);

void addPiece(Pieces_t type, int8_t x, int8_t y, int8_t z);
void movePiece(uint8_t idx, int8_t x, int8_t y, int8_t z);

void printBoardStatus();
