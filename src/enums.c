//
// Created by f3m on 11/04/25.
//
#include <stdint.h>

const int8_t directions[6][2] =
{
    //y   x
    {-1, 1}, //in alto a destra
    {0, 2},  //destra
    {1, 1}, //in basso a destra
    {1, -1}, //in basso a sinistra
    {0, -2}, //sinistra
    {-1, -1} //in alto a sinistra
};