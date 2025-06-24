//
// Created by f3m on 23/06/25.
//

#pragma once

#include "enums.h"

typedef struct Node
{
    Piece_t *move;
    Context_t *context;
    Node *childs;
    double score;
    int16_t cCount;
    bool isComplete;
    Piece_t *moves[15];
    uint64_t hash;
} Node_t;

typedef struct HashValue
{
    double score;
    bool isInTree;
    uint8_t depthCNN;
} HashValue_t;

typedef struct ThreadArgs
{
    uint8_t depth;
    Node_t *node;
} ThreadArgs_t;

int initTree();
void cleanTree();


int getBestChild();
int setRoot(Piece_t *move);
