//
// Created by f3m on 02/04/25.
//

#include "utils.h"

#include <moves.h>
#include <xxhash.h>

uint64_t hashPiece(const Pieces_t piece, const Pieces_t *neighbors)
{
    int_fast8_t max = 0;
    int_fast8_t idx = 0;
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const Pieces_t neighbor = neighbors[MtA(piece, i)];
        if (neighbor <= max)
            continue;

        max = neighbor;
        idx = i;
    }

    while (neighbors[MtA(piece, idx)] == max)
        idx--;


    uint64_t hash = 0;
    for (int i = 0; i < 6; ++i)
    {
        hash = hash << 8;
        hash += neighbors[MtA(piece, i)];
    }

    return hash;
}
uint64_t hashAll(const Pieces_t *neighbors)
{

    uint64_t toHash[28];

    for (int i = 0; i < 28; ++i)
        toHash[i] = hashPiece(i, neighbors);

    return XXH3_64bits(toHash, 28 * sizeof(uint64_t));
}

