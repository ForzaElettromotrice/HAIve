//
// Created by minga on 06/01/2025.
//

#pragma once

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "enums.h"
#include "logger.h"

typedef struct Piece
{
    Pieces_t type;
    Colors_t color;
    uint8_t x;
    uint8_t y;
    uint8_t z;
} Piece_t;

typedef struct Board
{
    uint8_t piecesCount;
    Piece_t board[28];
} Board_t;


int initGame();
void cleanGame();

bool isEncodingValid(const char *encoding, int *move, bool *add);
bool isMoveValid(const int *move, int *idx, bool add);

void addPiece(const Pieces_t type, const uint8_t x, const uint8_t y, const uint8_t z);
void movePiece(int idx, const int *newPos);

void printBoardStatus();
