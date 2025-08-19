//
// Created by f3m on 18/06/25.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <enums.h>

#define MOVES_ARRAYS 15
#define MOVES_SIZE 400

void getMoves(const HAIveContext_t *context, Piece_t *moves);
bool hasNeighbor(const Piece_t *piece, const Pieces_t *board);

#ifdef __cplusplus
}
#endif
