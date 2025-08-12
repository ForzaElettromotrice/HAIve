//
// Created by f3m on 11/04/25.
//
#include <stdint.h>
#include <enums.h>

const int8_t directions[6][2] =
{
    //y   x
    {-1, 1}, //in alto a destra
    {0, 2}, //destra
    {1, 1}, //in basso a destra
    {1, -1}, //in basso a sinistra
    {0, -2}, //sinistra
    {-1, -1} //in alto a sinistra
};

bool equalsPosition(const Position_t *p1, const Position_t *p2)
{
    return p1->x == p2->x && p1->y == p2->y && p1->z == p2->z;
}
bool equalsPiece(const Piece_t *p1, const Piece_t *p2)
{
    if (p1 == NULL || p2 == NULL)
        return false;
    return p1->id == p2->id && equalsPosition(&p1->position, &p2->position);
}
