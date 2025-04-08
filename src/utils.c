//
// Created by f3m on 02/04/25.
//
#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"


//TODO: correggi gli errori dei casi limite (e metti il modulo quando sottrai l'idx)
uint64_t hashPiece(const Pieces_t piece, const Pieces_t *neighbors)
{
    int_fast8_t max = 0;
    int_fast8_t idx = 0;
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const Pieces_t neighbor = neighbors[MtA(piece, i)];
        if (neighbor <= max)
            continue;

        max = neighbor;
        idx = i;
    }

    while (neighbors[MtA(piece, idx)] == max)
        idx--;


    uint64_t hash = 0;
    for (int i = 0; i < 6; ++i)
    {
        hash = hash << 8;
        hash += neighbors[MtA(piece, i)];
    }

    return hash;
}

//TODO: usa la matrice invece che la lista di adiacenza
uint64_t hashAll(const Pieces_t *neighbors)
{
    uint64_t toHash[28];

    for (int i = 0; i < 28; ++i)
        toHash[i] = hashPiece(i, neighbors);

    return XXH3_64bits(toHash, 28 * sizeof(uint64_t));
}


// Dalla stringa, ritorna il valore del pezzo
Pieces_t getPiece(const char *piece, char white)
{
    if (strcmp(piece, "Q") == 0)
    {
        return white ? W_QUEEN : B_QUEEN;
    }
    if (strcmp(piece, "S1") == 0)
    {
        return white ? W_SPIDER_1 : B_SPIDER_1;
    }
    if (strcmp(piece, "S2") == 0)
    {
        return white ? W_SPIDER_2 : B_SPIDER_2;
    }
    if (strcmp(piece, "G1") == 0)
    {
        return white ? W_GRASSHOPPER_1 : B_GRASSHOPPER_1;
    }
    if (strcmp(piece, "G2") == 0)
    {
        return white ? W_GRASSHOPPER_2 : B_GRASSHOPPER_2;
    }
    if (strcmp(piece, "G3") == 0)
    {
        return white ? W_GRASSHOPPER_3 : B_GRASSHOPPER_3;
    }
    if (strcmp(piece, "A1") == 0)
    {
        return white ? W_ANT_1 : B_ANT_1;
    }
    if (strcmp(piece, "A2") == 0)
    {
        return white ? W_ANT_2 : B_ANT_2;
    }
    if (strcmp(piece, "A3") == 0)
    {
        return white ? W_ANT_3 : B_ANT_3;
    }
    if (strcmp(piece, "B1") == 0)
    {
        return white ? W_BEETLE_1 : B_BEETLE_1;
    }
    if (strcmp(piece, "B2") == 0)
    {
        return white ? W_BEETLE_2 : B_BEETLE_2;
    }
    if (strcmp(piece, "L") == 0)
    {
        return white ? W_LADYBUG : B_LADYBUG;
    }
    if (strcmp(piece, "M") == 0)
    {
        return white ? W_MOSQUITO : B_MOSQUITO;
    }
    if (strcmp(piece, "P") == 0)
    {
        return white ? W_PILLBUG : B_PILLBUG;
    }

    return NULLPIECE; // Default case for invalid input
}

Pieces_t *convertFromMZinga(char *mzinga_string)
{
    char mosquito, ladybug, pillbug; // Dice quali pezzi sono presenti nel gioco
    mosquito = ladybug = pillbug = 0;
    char white; // true se noi siamo bianchi
    int turn; // Il numero del nostro turno
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

    // Get GameTypeString
    char *token = strtok(mzinga_string, ";");
    if (token == NULL) return NULL;

    if (strcmp(token, "Base") == 0)
    {
    } else if (strcmp(token, "Base+M") == 0)
    {
        mosquito = 1;
    } else if (strcmp(token, "Base+L") == 0)
    {
        ladybug = 1;
    } else if (strcmp(token, "Base+P") == 0)
    {
        pillbug = 1;
    } else if (strcmp(token, "Base+ML") == 0)
    {
        mosquito = ladybug = 1;
    } else if (strcmp(token, "Base+MP") == 0)
    {
        mosquito = pillbug = 1;
    } else if (strcmp(token, "Base+LP") == 0)
    {
        ladybug = pillbug = 1;
    } else if (strcmp(token, "Base+MLP") == 0)
    {
        mosquito = ladybug = pillbug = 1;
    } else
    {
        return NULL; // Invalid case
    }

    Pieces_t *pieces = malloc(sizeof(Pieces_t) * board_size);
    pieces = (Pieces_t *) memset(pieces, -1, sizeof(Pieces_t));
    if (pieces == NULL) return NULL;

    // Get GameStateString
    token = strtok(NULL, ";");
    if (strcmp(token, "NotStarted") == 0)
    {
        return pieces;
    }
    if (strcmp(token, "InProgress") == 0)
    {
        // In Progress
    } else
    {
        // Draw, White Won, Black Won
        return NULL;
    }

    // Get TurnString: Black[n] or White[n]
    token = strtok(NULL, ";");
    if (token[0] == 'B')
        white = 0;
    else
        white = 1;

    token[strlen(token) - 1] = '\0';
    turn = (int) strtol(token + 5, NULL, 10);

    char white_piece;
    Pieces_t piece;
    Position_t *id_to_pos = calloc(sizeof(Position_t), numPieces);

    // Il primo pezzo si gestisce fuori dal while
    token = strtok(NULL, ";");
    if (token[0] == 'b')
        white_piece = 0;
    else
        white_piece = 1;

    token++;
    piece = getPiece(token, white_piece);
    // NOTA: Io lo metto in (0, 0, 0) ma non è il centro della board
    id_to_pos[piece].x = 0;
    id_to_pos[piece].y = 0;
    id_to_pos[piece].z = 0;
    pieces[MtA(0, 0, 0)] = piece;

    while ((token = strtok(NULL, ";")) != NULL)
    {
        if (strcmp(token, "pass"))
            continue;

        char *space_pos = strchr(token, ' ');
        if (space_pos != NULL) *space_pos = '\0';
        if (token[0] == 'w')
            white_piece = 1;
        else
            white_piece = 0;
        token++;
        piece = getPiece(token, white_piece);
        if (id_to_pos[piece].x != 0 || id_to_pos[piece].y != 0 || id_to_pos[piece].z != 0)
        {
            pieces[
                MtA(id_to_pos[piece].x, id_to_pos[piece].y, id_to_pos[piece].z)
            ] = NULLPIECE;
        }

        Pieces_t other_piece;
        uint8_t direction[2] = {0, 0};
        token += strlen(token);
        token++;
        if (token[0] == '-')
        {
            direction[0] = directions[4][0];
            direction[1] = directions[4][1];
            white_piece = token[1] == 'w' ? 1 : 0;
            token++;
        } else if (token[0] == '/')
        {
            direction[0] = directions[3][0];
            direction[1] = directions[3][1];
            white_piece = token[1] == 'w' ? 1 : 0;
            token++;
        } else if (token[0] == '\\')
        {
            direction[0] = directions[5][0];
            direction[1] = directions[5][1];
            white_piece = token[1] == 'w' ? 1 : 0;
            token++;
        } else if (token[0] == 'w')
            white_piece = 1;
        else
            white_piece = 0;
        token++;

        if (direction[0] == 0 && direction[1] == 0)
        {
            char last_char = token[strlen(token) - 1];
            if (last_char == '-')
            {
                direction[0] = directions[1][0];
                direction[1] = directions[1][1];
            } else if (last_char == '\\')
            {
                direction[0] = directions[2][0];
                direction[1] = directions[2][1];
            } else
            {
                direction[0] = directions[0][0];
                direction[1] = directions[0][1];
            }
            token[strlen(token) - 1] = '\0';
        }
        other_piece = getPiece(token, white_piece);
        char x, y, z;
        x = id_to_pos[other_piece].x + direction[0];
        id_to_pos[piece].x = x;
        y = id_to_pos[other_piece].y + direction[1];
        id_to_pos[piece].y = y;
        z = id_to_pos[other_piece].z + (direction[0] == 0 && direction[1] == 0) ? 1 : 0;
        id_to_pos[piece].z = z;

        pieces[MtA(x, y, z)] = piece;
    }

    return pieces;
}
