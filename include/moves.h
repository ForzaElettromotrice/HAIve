//
// Created by f3m on 18/06/25.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <enums.h>

#define MOVES_ARRAYS 15
#define MOVES_SIZE 800
//Moves Matrix to Array
#define MMtA(piece, idx) ((piece) * MOVES_SIZE + (idx))

void getMoves(const HAIveContext_t *context, Piece_t *moves);

#ifdef __cplusplus
}
#endif
