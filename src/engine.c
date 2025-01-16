//
// Created by minga on 16/01/2025.
//

#include "engine.h"

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

uint8_t piecesCount;
Piece_t board[28];
bool boardGraph[28][28];
uint8_t *whitePiecesCount = NULL;
uint8_t *blackPiecesCount = NULL;
Colors_t colorTurn = NULLCOLOR;
bool firstMove = false;

Piece_t *findPiece(const int8_t x, const int8_t y, const int8_t z, uint8_t *idx)
{
    for (uint8_t i = 0; i < piecesCount; i++)
    {
        Piece_t *piece = &board[i];
        if (piece->x == x && piece->y == y && piece->z == z)
        {
            *idx = i;
            return piece;
        }
    }
    *idx = 255;
    return NULL;
}
bool hasPiece(const Pieces_t type)
{
    if (colorTurn == WHITE)
        return whitePiecesCount[type] > 0;
    return blackPiecesCount[type] > 0;
}
bool hasNeighbor(const int8_t x, const int8_t y, const int8_t z)
{
    if (z > 0)
    {
        uint8_t idx;
        findPiece(x, y, z - 1, &idx);
        return idx != 255;
    }

    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        uint8_t idx;
        findPiece(offX + x, offY + y, z, &idx);
        if (idx != 255)
            return true;
    }
    return false;
}
bool dfs(const uint8_t node, bool *visited)
{
    bool toFree = false;
    if (visited == NULL)
    {
        toFree = true;
        visited = calloc(piecesCount, sizeof(bool));
        if (!visited)
        {
            E_Print("malloc: %s\n", strerror(errno));
            return false;
        }
    }

    visited[node] = true;
    bool *neighbors = boardGraph[node];
    for (int i = 0; i < piecesCount; ++i)
    {
        if (neighbors[i] && !visited[i])
            dfs(i, visited);
    }


    if (toFree)
    {
        bool out = true;
        for (int i = 0; i < piecesCount; ++i)
        {
            if (!visited[i])
            {
                out = false;
                break;
            }
        }
        free(visited);
        return out;
    }
    return false;
}

int initGame()
{
    memset(board, 0, sizeof(board));
    memset(boardGraph, 0, sizeof(boardGraph));
    whitePiecesCount = malloc(8 * sizeof(uint8_t));
    blackPiecesCount = malloc(8 * sizeof(uint8_t));
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
    free(whitePiecesCount);
    free(blackPiecesCount);
    colorTurn = NULLCOLOR;
}

bool isEncodingValid(const char *encoding, int8_t *move, bool *add)
{
    char *copy = strdup(encoding);
    if (!copy)
    {
        E_Print("strdup: %s\n", strerror(errno));
        return false;
    }

    int i = 0;
    for (const char *token = strtok(copy, ","); token != NULL; token = strtok(NULL, ","))
    {
        move[i++] = (int8_t) strtol(token, NULL, 10);
    }


    if (i != 6 && i != 4)
    {
        free(copy);
        return false;
    }

    *add = i == 4;

    free(copy);
    return true;
}
bool isMoveValid(const int8_t *move, uint8_t *idx, const bool add)
{
    const int8_t x = move[0];
    const int8_t y = move[1];
    const int8_t z = move[2];
    if (firstMove && add && x == 0 && y == 0 && z == 0)
    {
        firstMove = false;
        return true;
    }

    const Piece_t *piece = findPiece(x, y, z, idx);

    if (add)
    {
        const Pieces_t type = (unsigned char) move[3];
        if (*idx != 255) //Se la posizione è occupata
            return false;
        if (!hasPiece(type)) //Se il player ha il pezzo disponibile in riserva
            return false;
        if (!hasNeighbor(x, y, z)) //Se il pezzo è posizionato vicino ad un altro
            return false;
        return true;
    }

    const int8_t dx = move[3];
    const int8_t dy = move[4];
    const int8_t dz = move[5];

    if (*idx == 255) //Se il pezzo esiste
        return false;

    if (piece->color != colorTurn) //Se il pezzo è del giocatore che fa la mossa
        return false;

    if (!hasNeighbor(dx, dy, dz)) //Se alla destinazione il pezzo avrà dei vicini
        return false;

    //TODO: fare si che il pezzo venga "rimosso" e si controlla se la board è divisa

    return true;
}


void movePiece(const uint8_t idx, const int8_t x, const int8_t y, const int8_t z)
{
    Piece_t *piece = &board[idx];

    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        uint8_t idxNeighbor;
        findPiece((int8_t) (offX + piece->x), (int8_t) (offY + piece->y), piece->z, &idxNeighbor);
        if (idxNeighbor == 255)
            continue;
        boardGraph[idx][idxNeighbor] = false;
        boardGraph[idxNeighbor][idx] = false;
    }
    piece->x = x;
    piece->y = y;
    piece->z = z;

    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        uint8_t idxNeighbor;
        findPiece((int8_t) (offX + piece->x), (int8_t) (offY + piece->y), piece->z, &idxNeighbor);
        if (idxNeighbor == 255)
            continue;
        boardGraph[idx][idxNeighbor] = true;
        boardGraph[idxNeighbor][idx] = true;
    }
}

void printBoardStatus()
{
    if (piecesCount == 0)
        printf("Board Empty!\n");
    for (int i = 0; i < piecesCount; ++i)
    {
        Piece_t piece = board[i];
        printf("%d - %d,%d,%d - %s\n", i, piece.x, piece.y, piece.z, piece.color == WHITE ? "white" : "black");
    }
}
