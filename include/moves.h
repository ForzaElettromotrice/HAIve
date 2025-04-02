//
// Created by f3m on 28/03/25.
//

#pragma once

#include "enums.h"

#define MtA(y,x) y * 28 + x

void queenMoves(const Piece_t *piece, const Pieces_t *neighbors, Position_t *moves, const uint8_t *mSize);
