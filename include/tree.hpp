//
// Created by f3m on 23/06/25.
//

#pragma once

#include "enums.h"

#define THREADS_NUM 16
#define BATCH_NUM 16
//TODO: da vedere, per ora ho messo 1 secondo
#define MIN_T 1.0

typedef struct Node
{
    const Piece_t *move;
    Piece_t *moves;
    Node **childs;
    uint64_t hash;
    double score;
    Context_t context;
    int16_t cCount;
    Node *bestChoice;
    bool outOfTree;
} Node_t;

typedef struct HashValue
{
    double score;
    Piece_t *moves;
} HashValue_t;

typedef struct BatchContext
{
    Node_t *nodes[BATCH_NUM];
    uint8_t count;
    double result[BATCH_NUM];
} BatchContext_t;

int initTree();
void cleanTree();


const Piece_t *getBestChild();
int setRoot(Piece_t *move);
