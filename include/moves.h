//
// Created by f3m on 18/06/25.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <enums.h>
#include <stdint.h>
#include <hashmap.h>

#define MOVES_ARRAYS 15
#define MOVES_SIZE 140
//Moves Matrix to Array
#define MMtA(piece, idx) ((piece) * MOVES_SIZE + (idx))
void *addMoves(void *arguments);

void queenMoves(Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx);

void beetleMoves(Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx);

void grasshopperMoves(Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx);

void pillbugMoves(Pieces_t id, const Position_t *position, const Context_t *context, Piece_t *moves, uint_fast8_t *idx);

void ladybugMoves(Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx);

void spiderMoves(Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx);

void antMoves(Pieces_t id, const Position_t *position, const Pieces_t *board, Piece_t *moves, uint_fast8_t *idx, Hashmap_t *visited);

void *mosquitoMoves(void *arguments);


void getMoves(const Context_t *context, Piece_t **moves_ptr);
void freeMoves(Piece_t **moves);

#ifdef __cplusplus
}
#endif
