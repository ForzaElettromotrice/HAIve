#pragma once

#include <stdbool.h>
#include <enums.h>
#include <math.h>
#include <stdio.h>
#include <moves.h>
#include <utils.h>

float negamax(Context_t* context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t* bestMove);