//
// Created by f3m on 28/03/25.
//

#pragma once

#include <stdbool.h>
#include <errno.h>
#include <hashmap.h>


#include "logger.h"
#include "enums.h"
#include "utils.h"


void getMoves(Pieces_t *board, const Position_t *positions, Colors_t color, Pieces_t last, Piece_t **moves, uint_fast8_t *mSize);