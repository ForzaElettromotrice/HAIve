//
// Created by f3m on 02/04/25.
//

#pragma once

#include <stdlib.h>
#include <string.h>
#include <xxhash.h>

#include "enums.h"


#define board_size 28 * 56 * 5

#define MtA(z,y,x) (z) * 56 * 28 + (y) * 28 + (x)


uint64_t hashAll(const Pieces_t *board, const Position_t *positions);
