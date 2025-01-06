//
// Created by minga on 06/01/2025.
//

#pragma once

#include <stdint.h>
#include <errno.h>

#include "logger.h"

typedef struct Piece
{
    uint8_t type;
    uint8_t x;
    uint8_t y;
    uint8_t z;
}Piece_t;

typedef struct Board
{
    uint8_t piecesCount;
    Piece_t board[28];
}Board_t;

int initGame(Board_t **board);
void cleanGame(Board_t *board);