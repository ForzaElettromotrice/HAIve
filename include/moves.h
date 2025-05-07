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


void getMoves(const Context_t *context, Piece_t **moves, uint_fast8_t *mSize);


void queenMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize);
void beetleMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize);
void grasshopperMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize);
void pillbugMoves(const Piece_t *piece, const Pieces_t *board,bool *visited, const Position_t *positions, const Pieces_t last, Piece_t *moves, uint_fast8_t *mSize);
void ladybugMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize);
void spiderMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize);
void antMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize, Hashmap_t *visited);
void mosquitoMoves(const Piece_t *piece, const Pieces_t *board,bool *visited, const Position_t *positions, const bool last, Piece_t *moves, uint_fast8_t *mSize);
void addMoves(const Context_t *context, Piece_t *moves, uint_fast8_t *mSize);
