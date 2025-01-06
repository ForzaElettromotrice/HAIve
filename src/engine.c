//
// Created by minga on 06/01/2025.
//

#include "engine.h"

#include <stdlib.h>
#include <string.h>


int initGame(Board_t **board)
{
    *board = calloc(1, sizeof(Board_t));
    if(!*board)
    {
        E_Print("malloc: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


void cleanGame(Board_t *board)
{
    free(board);
}