//
// Created by minga on 06/01/2025.
//

#include "engine.h"

#include <stdlib.h>
#include <string.h>

// int compare(const void *a, const void *b)
// {
//     const Piece_t *first = (Piece_t *) a;
//     const Piece_t *second= (Piece_t *) b;
//
//     // int diff = first->type - second->type;
//     // if (diff == 0) diff = first->x - second->x;
//     int diff = first->x - second->x;
//     if (diff == 0) diff = first->y - second->y;
//     if (diff == 0) diff = first->z - second->z;
//
//     return diff;
// }

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

bool isEncodingValid(const char *encoding, int *move)
{
    char *copy = strdup(encoding);

    int i = 0;
    for(const char *token = strtok(copy, ","); token != NULL; token = strtok(NULL, ","))
    {
        if(i >= 6)
            return false;
        move[i] = strtol(token, NULL, 10);
        i++;
    }
    if(i != 6)
        return false;
    free(copy);
    return true;
}
bool isMoveValid(const int *move, Board_t *board, int *idx)
{
    // Piece_t toMove = {NULLPIECE, move[0], move[1], move[2]};
    // Piece_t *found = bsearch(&toMove, board->board, board->piecesCount, sizeof(Piece_t), compare);
    Piece_t *found = NULL;
    for (int i = 0; i < board->piecesCount; i++)
    {
        Piece_t *piece = &board->board[i];
        if (piece->x == move[0] && piece->y == move[1] && piece->z == move[2])
        {
            found = piece;
            *idx = i;
            break;
        }
    }

    if(!found)
    {
        //TODO: controllare se si può aggiungere
        return false;
    }

    //TODO: controllare la destinazione
    //TODO: controllare il percorso

    return true;
}