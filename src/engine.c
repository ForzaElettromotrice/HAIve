//
// Created by minga on 16/01/2025.
//

#include "engine.h"

const uint8_t defaultPieces[] = {1, 1, 1, 1, 3, 3, 2, 2};
const int8_t directions[6][2] =
{
    {0, -2}, //sopra
    {-1, 1}, //in alto a destra
    {1, 1}, //in basso a destra
    {0, 2}, //sotto
    {1, -1}, //in basso a sinistra
    {-1, -1} //in alto a sinistra
};

uint8_t piecesCount;
Piece_t board[28];
bool boardGraph[28][28];

uint8_t whitePiecesCount[8];
uint8_t blackPiecesCount[8];

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
        findPiece(x, y, (int8_t) (z - 1), &idx);
        return idx != 255;
    }

    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        uint8_t idx;
        findPiece((int8_t) (offX + x), (int8_t) (offY + y), z, &idx);
        if (idx != 255)
            return true;
    }
    return false;
}
bool dfs(const uint8_t node, bool *visited, const uint8_t removedOne)
{
    bool toFree = false;
    if (visited == NULL)
    {
        toFree = true;
        visited = calloc(piecesCount, sizeof(bool));
        if (!visited)
        {
            E_Print("calloc: %s\n", strerror(errno));
            return false;
        }
        visited[removedOne] = true;
    }

    visited[node] = true;
    const bool *neighbors = boardGraph[node];
    for (int i = 0; i < piecesCount; ++i)
    {
        if (neighbors[i] && !visited[i])
            dfs(i, visited, removedOne);
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
void removePiece(const uint8_t node)
{
    const Piece_t *piece = &board[node];
    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        uint8_t idxNeighbor;
        findPiece((int8_t) (offX + piece->x), (int8_t) (offY + piece->y), piece->z, &idxNeighbor);
        if (idxNeighbor == 255)
            continue;
        boardGraph[node][idxNeighbor] = false;
        boardGraph[idxNeighbor][node] = false;
    }
}
void readdPiece(const Piece_t *piece, const uint8_t node)
{
    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        uint8_t idxNeighbor;
        findPiece((int8_t) (offX + piece->x), (int8_t) (offY + piece->y), piece->z, &idxNeighbor);
        if (idxNeighbor == 255)
            continue;
        boardGraph[node][idxNeighbor] = true;
        boardGraph[idxNeighbor][node] = true;
    }
}

bool isBlocked(const int8_t direction, const int8_t x, const int8_t y, const int8_t z)
{
    int8_t d = (int8_t) (direction - 1 % 6);
    int8_t offX = directions[d][0];
    int8_t offY = directions[d][1];
    uint8_t idx;
    findPiece((int8_t) (x + offX), (int8_t) (y + offY), z, &idx);

    if (idx == 255)
        return false;

    d = (int8_t) (direction + 1 % 6);
    offX = directions[d][0];
    offY = directions[d][1];
    findPiece((int8_t) (x + offX), (int8_t) (y + offY), z, &idx);

    return idx != 255;
}

bool queenRoute(const int8_t sx, const int8_t sy, const int8_t ex, const int8_t ey)
{
    for (int8_t i = 0; i < 6; ++i)
    {
        const int8_t newX = (int8_t) (directions[i][0] + sx);
        const int8_t newY = (int8_t) (directions[i][1] + sy);
        uint8_t idx;
        findPiece(newX, newY, 0, &idx);
        if (idx == 255)
            continue;
        if (!isBlocked(i, newX, newY, 0))
            continue;
        if (newX == ex && newY == ey)
            return true;
    }
    return false;
}
bool beetleRoute(const int8_t sx, const int8_t sy, const int8_t sz, const int8_t ex, const int8_t ey, const int8_t ez)
{
    for (int8_t i = 0; i < 6; ++i)
    {
        const int8_t newX = (int8_t) (directions[i][0] + sx);
        const int8_t newY = (int8_t) (directions[i][1] + sy);

        uint8_t idx;
        findPiece(newX, newY, ez, &idx);
        if (idx == 255)
            continue;
        if (!isBlocked(i, newX, newY, ez))
            continue;
        if (newX == ex && newY == ey)
            return true;
    }

    return false;
}
bool grasshopperRoute(const int8_t sx, const int8_t sy, const int8_t ex, const int8_t ey)
{
    int8_t dx;
    if (sx - ex > 0)
        dx = 1;
    else if (sx - ex == 0)
        dx = 0;
    else
        dx = -1;
    const int8_t dy = sy - ey > 0 ? 2 : (sy - ey == 0 ? 0 : -2);


    uint8_t idx;
    findPiece(sx + dx, sy + dy, 0, &idx);
    if (idx == 255)
        return false;
    int8_t j = sy + dy;
    for (int8_t i = sx + dx; i != ex; i += dx)
    {
        j += dy;

        if (i == ex && j == ey)
            return true;

        findPiece(i, j, 0, &idx);
        if (idx == 255)
            return false;
    }


    return false;
}

bool existRoute(const Pieces_t type, const int8_t sx, const int8_t sy, const int8_t sz, const int8_t ex, const int8_t ey, const int8_t ez)
{
    switch (type)
    {
        case NULLPIECE:
            E_Print("Invalid type piece!\n");
            return false;
        case QUEEN:
            return queenRoute(sx, sy, ex, ey);
        case PILLBUG:
            break;
        case LADYBUG:
            break;
        case MOSQUITO:
            break;
        case ANT:
            break;
        case GRASSHOPPER:
            return grasshopperRoute(sx, sy, ex, ey);
        case BEETLE:
            return beetleRoute(sx, sy, sz, ex, ey, ez);
        case SPIDER:
            break;
    }
    return false;
}


int initGame()
{
    memset(board, 0, sizeof(board));
    memset(boardGraph, 0, sizeof(boardGraph));

    memcpy(whitePiecesCount, defaultPieces, 8 * sizeof(uint8_t));
    memcpy(blackPiecesCount, defaultPieces, 8 * sizeof(uint8_t));

    colorTurn = WHITE;
    firstMove = true;

    return EXIT_SUCCESS;
}
void cleanGame()
{
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

    removePiece(*idx); //rimuovi temporaneamente
    if (!dfs(*idx != 0 ? 0 : 1, NULL, *idx)) //Se la board viene divisa in 2 senza quel pezzo
    {
        readdPiece(piece, *idx); // rimetti il pezzo dove era
        return false;
    }

    //TODO: vedere se esiste un percorso valido
    return true;
}

void addPiece(const Pieces_t type, const int8_t x, const int8_t y, const int8_t z)
{
    Piece_t *piece = &board[piecesCount++];
    piece->x = x;
    piece->y = y;
    piece->z = z;
    piece->color = colorTurn;
    piece->type = type;

    readdPiece(piece, piecesCount - 1);
    if (colorTurn == WHITE)
        whitePiecesCount[type]--;
    else
        blackPiecesCount[type]--;
    colorTurn *= -1;
}
void movePiece(const uint8_t idx, const int8_t x, const int8_t y, const int8_t z)
{
    Piece_t *piece = &board[idx];

    piece->x = x;
    piece->y = y;
    piece->z = z;

    readdPiece(piece, idx);
    colorTurn *= -1;
}

void printBoardStatus()
{
    if (piecesCount == 0)
        printf("Board Empty!\n");
    for (int i = 0; i < piecesCount; ++i)
    {
        Piece_t piece = board[i];
        printf("%d - %d,%d,%d - %d - %s\n", i, piece.x, piece.y, piece.z, piece.type, piece.color == WHITE ? "white" : "black");
    }
}
