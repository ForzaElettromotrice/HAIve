//
// Created by f3m on 28/03/25.
//

#pragma once

#include "enums.h"
#include "utils.h"


void queenMoves(const Piece_t *piece, const Pieces_t *board, Piece_t *moves, uint_fast8_t *mSize);


// INPUT:
//      1 - matrice che rappresenta la board
//      2 - pezzo
//OUTPUT:
//      1 - lista di mosse nello stile [(id,x,y,z)...] ->(ovvero la struct Piece_t)
//      2 - numero di mosse generate
