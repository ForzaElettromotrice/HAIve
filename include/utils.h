//
// Created by f3m on 02/04/25.
//

#pragma once

#include <stdlib.h>
#include <string.h>
#include <xxhash.h>

#include "enums.h"

// TODO: Giusto???
#define board_size 28 * 56 * 5

#define MtA(z,y,x) z * 56 * 28 + y * 28 + x


uint64_t hashPiece(Pieces_t piece, const Pieces_t *neighbors);
