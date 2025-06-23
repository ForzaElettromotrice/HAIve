//
// Created by f3m on 23/06/25.
//

#pragma once
#include <stdint.h>

#include "enums.h"

typedef struct Node
{
    double score;
    int16_t cCount;
    struct Node *childs;
    Context_t *context;
}Node_t;


int initTree();
void cleanTree();


int getBestChild();
int setRoot();