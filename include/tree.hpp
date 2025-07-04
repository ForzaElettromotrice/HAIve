//
// Created by f3m on 23/06/25.
//

#pragma once

#include "enums.h"

#define THREADS_NUM 16

typedef struct Node
{
    Piece_t *move;
    Piece_t *moves;
    Node *childs;
    uint64_t hash;
    double score;
    Context_t context;
    int16_t cCount;
} Node_t;

typedef struct HashValue
{
    double score;
} HashValue_t;

typedef struct BatchContext
{
    Context_t *contexts;
    uint_fast8_t size;
} BatchContext_t;


int initTree();
void cleanTree();


int getBestChild();
int setRoot(Piece_t *move);
