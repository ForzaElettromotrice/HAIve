//
// Created by minga on 06/01/2025.
//

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define NUM_PIECES 28
#define RIGHT_UP 0
#define RIGHT 1
#define RIGHT_DOWN 2
#define LEFT_DOWN 3
#define LEFT 4
#define LEFT_UP 5
extern const int8_t directions[6][2];

typedef enum Pieces
{
    NULLPIECE = -1,
    B_QUEEN,
    B_PILLBUG,
    B_LADYBUG,
    B_MOSQUITO,
    B_ANT_1,
    B_ANT_2,
    B_ANT_3,
    B_GRASSHOPPER_1,
    B_GRASSHOPPER_2,
    B_GRASSHOPPER_3,
    B_BEETLE_1,
    B_BEETLE_2,
    B_SPIDER_1,
    B_SPIDER_2,
    W_QUEEN,
    W_PILLBUG,
    W_LADYBUG,
    W_MOSQUITO,
    W_ANT_1,
    W_ANT_2,
    W_ANT_3,
    W_GRASSHOPPER_1,
    W_GRASSHOPPER_2,
    W_GRASSHOPPER_3,
    W_BEETLE_1,
    W_BEETLE_2,
    W_SPIDER_1,
    W_SPIDER_2,
} Pieces_t;


typedef enum Colors
{
    NULLCOLOR = 0,
    WHITE = 1,
    BLACK = -1
} Colors_t;

typedef struct Position
{
    int8_t z;
    int8_t y;
    int8_t x;
} Position_t;

typedef struct Piece
{
    Pieces_t id;
    Position_t position;
} Piece_t;

#pragma pack(1)
typedef struct GameType
{
    bool ladybug;
    bool pillbug;
    bool mosquito;
} GameType_t;

typedef enum GameStatus
{
    NOT_INITIALIZED = -1,
    NOT_STARTED,
    WHITE_WON,
    BLACK_WON,
    DRAW,
    IN_PROGRESS
} GameStatus_t;

typedef struct Context
{
    Pieces_t *board;
    char *moves;
    size_t movesSize;
    Position_t *idToPos;

    int16_t turn;
    Colors_t curColor;
    GameStatus_t gameStatus;
    GameType_t gameType;
    Pieces_t lastMovedPiece;
} Context_t;

/*
    Il contesto sarà il contesto MODIFICATO dalla mossa che muove il pezzo pieceMoved.
    Non serve sapere dov'era il pezzo prima: il nodo padre avrà il contesto originale.
*/
typedef struct Node
{
    struct Node *left;
    struct Node *right;
    Context_t *context;

    uint8_t child_number;
    Piece_t pieceMoved;
} Node_t;
