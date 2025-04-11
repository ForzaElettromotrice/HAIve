//
// Created by f3m on 11/04/25.
//
#include "enums.h"

const int8_t directions[6][2] =
{
    //y   x
    {-2, 0}, //sopra
    {-1, 1}, //in alto a destra
    {1, 1}, //in basso a destra
    {2, 0}, //sotto
    {1, -1}, //in basso a sinistra
    {-1, -1} //in alto a sinistra
};
