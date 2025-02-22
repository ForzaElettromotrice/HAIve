//
// Created by f3m on 22/02/25.
//

#include "engine.h"

const int8_t directions[6][2] =
{
    {0, -2}, //sopra
    {-1, 1}, //in alto a destra
    {1, 1}, //in basso a destra
    {0, 2}, //sotto
    {1, -1}, //in basso a sinistra
    {-1, -1} //in alto a sinistra
};

Piece_t board[28];
Pieces_t neighbors[28][6];

Colors_t colorTurn = NULLCOLOR;
bool firstMove = false;

bool isOccupied(const int8_t x, const int8_t y, const int8_t z, Pieces_t *id)
{
    Pieces_t tmp;
    Pieces_t *piece = &tmp;
    if (id != NULL)
        piece = id;
    for (uint8_t i = 0; i < 28; i++)
    {
        const Piece_t *p = &board[i];
        if (p->x == x && p->y == y && p->z == z)
        {
            *piece = p->id;
            return true;
        }
    }
    *piece = NULLPIECE;
    return false;
}
bool hasNeighbor(const int8_t x, const int8_t y, const int8_t z)
{
    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        if (isOccupied((int8_t) (x + offX), (int8_t) (y + offY), z, NULL))
            return true;
    }
    return false;
}
bool dfs(const Pieces_t id, bool *visited, const Pieces_t original)
{
    bool toFree = false;
    if (visited == NULL)
    {
        toFree = true;
        visited = calloc(28, sizeof(bool));
        if (!visited)
        {
            E_Print("calloc: %s\n", strerror(errno));
            return false;
        }
        visited[original] = true;
    }

    visited[id] = true;
    for (int i = 0; i < 6; ++i)
    {
        const Pieces_t neighbor = neighbors[id][i];
        if (neighbor == NULLPIECE || visited[neighbor])
            continue;
        dfs(neighbor, visited, original);
    }

    if (toFree)
    {
        bool out = true;
        for (int i = 0; i < 28; ++i)
            out &= visited[i];
        return out;
    }

    return false;
}
bool divideBoard(const Pieces_t id)
{
    //Rimuovo il pezzo da tutti
    for (int i = 0; i < 6; ++i)
    {
        const Pieces_t neighbor = neighbors[id][i];
        if (neighbor == NULLPIECE)
            continue;

        neighbors[neighbor][(i + 3) % 6] = NULLPIECE;
    }

    //non posso partire dal pezzo che rimuovo
    const Pieces_t entry = id == W_QUEEN ? B_QUEEN : W_QUEEN;

    //controllo se spezzo la board
    const bool result = !dfs(entry, NULL, id);

    //riaggiungo il pezzo
    for (int i = 0; i < 6; ++i)
    {
        const Pieces_t neighbor = neighbors[id][i];
        if (neighbor == NULLPIECE)
            continue;

        neighbors[neighbor][(i + 3) % 6] = id;
    }

    return result;
}

int initGame()
{
    memset(board, 0xff, sizeof(board));
    memset(neighbors, 0xff, sizeof(neighbors));

    colorTurn = WHITE;
    firstMove = true;

    return EXIT_SUCCESS;
}
void cleanGame()
{
    colorTurn = NULLCOLOR;
}

bool isEncodingValid(const char *encoding, int8_t *move)
{
    char *copy = strdup(encoding);
    if (!copy)
    {
        E_Print("strdup: %s\n", strerror(errno));
        return false;
    }

    int i = 0;
    for (const char *token = strtok(copy, ","); token != NULL; token = strtok(NULL, ","))
        move[i++] = (int8_t) strtol(token, NULL, 10);

    free(copy);
    return i == 4 && move[0] >= 0 && move[0] <= 27;
}
bool isMoveValid(const Pieces_t id, const int8_t x, const int8_t y, const int8_t z)
{
    if (firstMove)
    {
        if (x == 0 && y == 0 && z == 0 && id >= 14)
        {
            firstMove = false;
            return true;
        }
        return false;
    }

    const bool add = board[id].id == NULLPIECE; // se il pezzo è NULL vuol dire che va aggiunto

    if (add)
    {
        //TODO: il pezzo non può toccare pezzi del colore avversario (tranne per la prima mossa del nero)

        if ((id < 14) == (colorTurn == BLACK)) //se il pezzo appartiene al giocatore corrente...
            return false;
        if (isOccupied(x, y, z, NULL)) //se la posizione è libera...
            return false;
        if (!hasNeighbor(x, y, z)) //se ha almeno 1 vicino ...
            return false;
        return true; //allora ok!
    }

    if (isOccupied(x, y, z, NULL)) //se la posizione è libera...
        return false;

    if ((id < 14) == (colorTurn == BLACK)) //se il pezzo appartiene al giocatore corrente...
        return false;

    if (!hasNeighbor(x, y, z)) //se alla destinazione ha almeno 1 vicino...
        return false;

    if (!divideBoard(id)) //se togliendolo la board si spezzerebbe...
        return false;

    //TODO: controllare il percorso

    return false;
}

void doMove(const Pieces_t id, const int8_t x, const int8_t y, const int8_t z)
{
    //Rimuovo il pezzo da tutti
    for (int i = 0; i < 6; ++i)
    {
        const Pieces_t neighbor = neighbors[id][i];
        if (neighbor == NULLPIECE)
            continue;

        neighbors[id][i] = NULLPIECE;
        neighbors[neighbor][(i + 3) % 6] = NULLPIECE;
    }

    //aggiorno la posizione
    Piece_t *piece = &board[id];
    piece->id = id;
    piece->x = x;
    piece->y = y;
    piece->z = z;

    //aggiorno la matrice dei vicini
    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        Pieces_t neighbor;

        if (!isOccupied((int8_t) (x + offX), (int8_t) (y + offY), z, &neighbor))
            continue;

        neighbors[id][i] = neighbor;
        neighbors[neighbor][(i + 3) % 6] = id;
    }
}

void printBoardStatus()
{
    for (int i = 0; i < 28; ++i)
    {
        const Piece_t piece = board[i];
        printf("%d - %d,%d,%d\n", i, piece.x, piece.y, piece.z);
    }
}


// bool queenRoute(const int8_t sx, const int8_t sy, const int8_t ex, const int8_t ey)
// {
//     for (int8_t i = 0; i < 6; ++i)
//     {
//         const int8_t newX = (int8_t) (directions[i][0] + sx);
//         const int8_t newY = (int8_t) (directions[i][1] + sy);
//         uint8_t idx;
//         findPiece(newX, newY, 0, &idx);
//         if (idx == 255)
//             continue;
//         if (!isBlocked(i, newX, newY, 0))
//             continue;
//         if (newX == ex && newY == ey)
//             return true;
//     }
//     return false;
// }
// bool beetleRoute(const int8_t sx, const int8_t sy, const int8_t sz, const int8_t ex, const int8_t ey, const int8_t ez)
// {
//     for (int8_t i = 0; i < 6; ++i)
//     {
//         const int8_t newX = (int8_t) (directions[i][0] + sx);
//         const int8_t newY = (int8_t) (directions[i][1] + sy);
//
//         uint8_t idx;
//         findPiece(newX, newY, ez, &idx);
//         if (idx == 255)
//             continue;
//         if (!isBlocked(i, newX, newY, ez))
//             continue;
//         if (newX == ex && newY == ey)
//             return true;
//     }
//
//     return false;
// }
// bool grasshopperRoute(const int8_t sx, const int8_t sy, const int8_t ex, const int8_t ey)
// {
//     int8_t dx;
//     if (sx - ex > 0)
//         dx = 1;
//     else if (sx - ex == 0)
//         dx = 0;
//     else
//         dx = -1;
//     const int8_t dy = sy - ey > 0 ? 2 : (sy - ey == 0 ? 0 : -2);
//
//
//     uint8_t idx;
//     findPiece(sx + dx, sy + dy, 0, &idx);
//     if (idx == 255)
//         return false;
//     int8_t j = sy + dy;
//     for (int8_t i = sx + dx; i != ex; i += dx)
//     {
//         j += dy;
//
//         if (i == ex && j == ey)
//             return true;
//
//         findPiece(i, j, 0, &idx);
//         if (idx == 255)
//             return false;
//     }
//
//
//     return false;
// }
//
// bool existRoute(const Pieces_t type, const int8_t sx, const int8_t sy, const int8_t sz, const int8_t ex, const int8_t ey, const int8_t ez)
// {
//     switch (type)
//     {
//         case NULLPIECE:
//             E_Print("Invalid type piece!\n");
//             return false;
//         case QUEEN:
//             return queenRoute(sx, sy, ex, ey);
//         case PILLBUG:
//             break;
//         case LADYBUG:
//             break;
//         case MOSQUITO:
//             break;
//         case ANT:
//             break;
//         case GRASSHOPPER:
//             return grasshopperRoute(sx, sy, ex, ey);
//         case BEETLE:
//             return beetleRoute(sx, sy, sz, ex, ey, ez);
//         case SPIDER:
//             break;
//     }
//     return false;
// }
