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
bool isBlocked(const int8_t direction, const int8_t x, const int8_t y, const int8_t z)
{
    int8_t offX = directions[(direction - 1) % 6][0];
    int8_t offY = directions[(direction - 1) % 6][1];
    const bool result = isOccupied((int8_t) (x + offX), (int8_t) (y + offY), z, NULL);

    offX = directions[(direction + 1) % 6][0];
    offY = directions[(direction + 1) % 6][0];
    return result && isOccupied((int8_t) (x + offX), (int8_t) (y + offY), z, NULL);
}
bool queenRoute(const Pieces_t id, const int8_t fx, const int8_t fy)
{
    const Piece_t piece = board[id];
    const int8_t ix = piece.x;
    const int8_t iy = piece.y;

    for (int8_t i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        if (ix + offX == fx && iy + offY == fy)
            return !isBlocked(i, ix, iy, 0);
    }
    return false;
}
bool beetleRoute(const Pieces_t id, const int8_t fx, const int8_t fy, const int8_t fz)
{
    const Piece_t piece = board[id];
    const int8_t ix = piece.x;
    const int8_t iy = piece.y;
    const int8_t iz = piece.z;

    if (fz - iz > 1 || fz - iz < -1)
        return false;

    for (int8_t i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        if (ix + offX == fx && iy + offY == fy)
            return !isBlocked(i, ix, iy, fz);
    }
    return false;
}
bool grasshopperRoute(const Pieces_t id, const int8_t fx, const int8_t fy)
{
    const Piece_t piece = board[id];
    const int8_t ix = piece.x;
    const int8_t iy = piece.y;

    int8_t offX;
    int8_t offY;

    if (ix - fx == 0)
    {
        if (abs(iy - fy) % 2 != 0)
            return false;
        offX = 0;
        offY = fy - iy > 0 ? 2 : -2;
    } else
    {
        if (abs(ix - fx) != abs(iy - fy))
            return false;
        offX = fx - ix > 0 ? 1 : -1;
        offY = fy - iy > 0 ? 1 : -1;
    }

    int8_t currX = ix;
    int8_t currY = iy;
    while (currX != fx)
    {
        if (!isOccupied(currX, currY, 0, NULL))
            return false;
        currX += offX;
        currY += offY;
    }
    return true;
}
bool pillbugRoute(const Pieces_t id, const int8_t fx, const int8_t fy)
{
}
bool ladybugRoute(const Pieces_t id, const int8_t ix, const int8_t iy, const int8_t fx, const int8_t fy, const int8_t depth)
{
    for (int i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];
        const int8_t newX = (int8_t) (ix + offX);
        const int8_t newY = (int8_t) (iy + offY);

        if (depth == 3)
        {
            if (neighbors[id][i] == NULLPIECE)
                continue;

            if (isOccupied(newX, newY, 1, NULL))
                continue;

            if (ladybugRoute(id, newX, newY, fx, fy, 2))
                return true;
        }
        if (depth == 2)
        {
            if (isOccupied(newX, newY, 1, NULL))
            {
                if (!isOccupied(newX, newY, 2, NULL) && ladybugRoute(id, newX, newY, fx, fy, 1))
                    return true;
            } else if (isOccupied(newX, newY, 0, NULL) && ladybugRoute(id, newX, newY, fx, fy, 1))
                return true;
            continue;
        }
        if (depth == 1)
        {
            if (newX != fx || newY != fy)
                continue;
            if (!isOccupied(newX, newY, 0, NULL))
                return true;
        }
    }
    return false;
}
bool antRoute(const Pieces_t id, const int8_t fx, const int8_t fy)
{
}
bool spiderRoute(const int8_t ix, const int8_t iy, const int8_t fx, const int8_t fy, const int8_t depth)
{
    //TODO: vettore dei visitati
    if (depth == 0)
        return false;

    if (ix == fx && iy == fy)
        return true;

    for (int8_t i = 0; i < 6; ++i)
    {
        const int8_t offX = directions[i][0];
        const int8_t offY = directions[i][1];

        const int8_t newX = (int8_t) (ix + offX);
        const int8_t newY = (int8_t) (iy + offY);

        if (isOccupied(newX, newY, 0, NULL))
            continue;
        if (isBlocked(i, ix, iy, 0))
            continue;

        if (spiderRoute(newX, newY, fx, fy, (int8_t) (depth - 1)))
            return true;
    }
    return false;
}
bool mosquitoRoute(const Pieces_t id, const int8_t fx, const int8_t fy, const int8_t fz)
{
    const Piece_t piece = board[id];
    const int8_t ix = piece.x;
    const int8_t iy = piece.y;
    const int8_t iz = piece.z;

    if (iz > 0)
        return beetleRoute(id, fx, fy, fz);

    bool out = false;

    for (int i = 0; i < 6; ++i)
    {
        const Pieces_t neighbor = neighbors[id][i];
        switch (neighbor)
        {
            case B_QUEEN:
            case W_QUEEN:
                out |= queenRoute(id, fx, fy);
                break;
            case B_PILLBUG:
            case W_PILLBUG:
                out |= pillbugRoute(id, fx, fy);
                break;
            case B_LADYBUG:
            case W_LADYBUG:
                out |= ladybugRoute(id, ix, iy, fx, fy, 3);
                break;
            case B_MOSQUITO:
            case W_MOSQUITO:
                out |= false;
                break;
            case B_ANT_1:
            case B_ANT_2:
            case B_ANT_3:
            case W_ANT_1:
            case W_ANT_2:
            case W_ANT_3:
                out |= antRoute(id, fx, fy);
                break;
            case B_GRASSHOPPER_1:
            case B_GRASSHOPPER_2:
            case B_GRASSHOPPER_3:
            case W_GRASSHOPPER_1:
            case W_GRASSHOPPER_2:
            case W_GRASSHOPPER_3:
                out |= grasshopperRoute(id, fx, fy);
                break;
            case B_BEETLE_1:
            case B_BEETLE_2:
            case W_BEETLE_1:
            case W_BEETLE_2:
                out |= beetleRoute(id, fx, fy, fz);
                break;
            case B_SPIDER_1:
            case B_SPIDER_2:
            case W_SPIDER_1:
            case W_SPIDER_2:
                out |= spiderRoute(ix, iy, fx, fy, 3);
                break;
            case NULLPIECE:
                out |= false;
                break;
        }
        if (out)
            return out;
    }
    return false;
}
bool routeValid(const Pieces_t id, const int8_t x, const int8_t y, const int8_t z)
{
    const Piece_t piece = board[id];
    const int8_t ix = piece.x;
    const int8_t iy = piece.y;

    switch (id)
    {
        case B_QUEEN:
        case W_QUEEN:
            return queenRoute(id, x, y);
        case B_PILLBUG:
        case W_PILLBUG:
            break;
        case B_LADYBUG:
        case W_LADYBUG:
            if (z != 0)
                return false;
            return ladybugRoute(id, ix, iy, x, y, 3);
        case B_MOSQUITO:
        case W_MOSQUITO:
            break;
        case B_ANT_1:
        case B_ANT_2:
        case B_ANT_3:
        case W_ANT_1:
        case W_ANT_2:
        case W_ANT_3:
            break;
        case B_GRASSHOPPER_1:
        case B_GRASSHOPPER_2:
        case B_GRASSHOPPER_3:
        case W_GRASSHOPPER_1:
        case W_GRASSHOPPER_2:
        case W_GRASSHOPPER_3:
            return grasshopperRoute(id, x, y);
        case B_BEETLE_1:
        case B_BEETLE_2:
        case W_BEETLE_1:
        case W_BEETLE_2:
            return beetleRoute(id, x, y, z);
        case B_SPIDER_1:
        case B_SPIDER_2:
        case W_SPIDER_1:
        case W_SPIDER_2:
            return spiderRoute(ix, iy, x, y, 3);
        case NULLPIECE:
            break;
    }
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

    if (!routeValid(id, x, y, z)) //se il percorso è valido
        return false;

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

