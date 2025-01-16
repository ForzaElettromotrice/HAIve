//
// Created by minga on 06/01/2025.
//

#include "oldEngine.h"

#include <stdlib.h>
#include <string.h>

const uint8_t defaultPieces[] = {1, 1, 1, 1, 3, 3, 2, 2};
const int8_t directions[6][2] =
{
    {-1, -1}, //in alto a sinistra
    {-1, 1}, //in alto a destra
    {0, -2}, //sopra
    {0, 2}, //sotto
    {1, -1}, //in basso a sinistra
    {1, 1} //in basso a destra
};

Board_t *board = NULL;
uint8_t *whitePiecesCount = NULL;
uint8_t *blackPiecesCount = NULL;
Colors_t colorTurn = NULLCOLOR;
bool firstMove = false;


int initGame()
{
    board = calloc(1, sizeof(Board_t));
    if (!board)
    {
        E_Print("malloc: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    whitePiecesCount = malloc(8 * sizeof(int));
    blackPiecesCount = malloc(8 * sizeof(int));
    if (!whitePiecesCount || !blackPiecesCount)
    {
        E_Print("malloc: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    memcpy(whitePiecesCount, defaultPieces, 8 * sizeof(uint8_t));
    memcpy(blackPiecesCount, defaultPieces, 8 * sizeof(uint8_t));

    colorTurn = WHITE;
    firstMove = true;

    return EXIT_SUCCESS;
}
void cleanGame()
{
    free(board);
    free(whitePiecesCount);
    free(blackPiecesCount);
    colorTurn = NULLCOLOR;
}

bool haveNeighbors(const int x, const int y, const int z)
{
    if (z > 0)
    {
        bool found = false;
        for (int i = 0; i < board->piecesCount; ++i)
        {
            Piece_t piece = board->board[i];
            if (piece.x == x && piece.y == y && piece.z == z - 1)
                found = true;
        }
        return found;
    }
    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        for (int j = 0; j < board->piecesCount; ++j)
        {
            const Piece_t piece = board->board[j];
            if (piece.x == x + offX && piece.y == y + offY)
                return true;
        }
    }
    return false;
}

bool canAddPiece(const int *move)
{
    if (move[3] < 0 || move[3] > 8)
        return false;

    if (colorTurn == WHITE)
        return whitePiecesCount[move[3]] > 0 && haveNeighbors(move[0], move[1], move[2]);
    return blackPiecesCount[move[3]] > 0 && haveNeighbors(move[0], move[1], move[2]);
}
bool isEncodingValid(const char *encoding, int *move, bool *add)
{
    char *copy = strdup(encoding);
    if (!copy)
    {
        E_Print("strdup: %s\n", strerror(errno));
        return false;
    }

    int i = 0;
    for (const char *token = strtok(copy, ","); token != NULL; token = strtok(NULL, ","))
        move[i++] = strtol(token, NULL, 10);


    if (i != 6 && i != 4)
    {
        free(copy);
        return false;
    }

    *add = i == 4;

    free(copy);
    return true;
}
bool isMoveValid(const int *move, int *idx, bool add)
{
    if (firstMove && move[0] == 0 && move[1] == 0 && move[2] == 0)
    {
        firstMove = false;
        return true;
    }

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

    if (!found)
    {
        if (!add)
            return false;
        if (canAddPiece(move))
            return true;
        return false;
    }

    if (add)
    {
        return false;
    }

    if (found->color != colorTurn)
        return false;

    //TODO: controllare se spezza in 2 la board
    //TODO: controllare la destinazione
    //TODO: controllare il percorso

    return true;
}


void addPiece(const Pieces_t type, const uint8_t x, const uint8_t y, const uint8_t z)
{
    const Piece_t piece = {type, colorTurn, x, y, z};
    memcpy(&board->board[board->piecesCount], &piece, sizeof(Piece_t));
    board->piecesCount++;
    if (colorTurn == WHITE)
        whitePiecesCount[type]--;
    else
        blackPiecesCount[type]--;

    colorTurn *= -1; //Passa il turno al giocatore successivo
}
void movePiece(int idx, const int *newPos)
{
    Piece_t *piece = &board->board[idx];
    piece->x = newPos[0];
    piece->y = newPos[1];
    piece->z = newPos[2];

    colorTurn *= -1; //Passa il turno al giocatore successivo
}

void printBoardStatus()
{
    if (board->piecesCount == 0)
        printf("Board Empty!\n");
    for (int i = 0; i < board->piecesCount; ++i)
    {
        Piece_t piece = board->board[i];
        printf("%d - %d,%d,%d - %s\n", i, piece.x, piece.y, piece.z, piece.color == WHITE ? "white" : "black");
    }
}
