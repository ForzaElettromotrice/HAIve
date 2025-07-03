//
// Created by f3m on 03/07/25.
//

#pragma once

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

#define LEVELS 10

typedef struct HNode
{
    void *val;
    struct HNode *next;
} HNode_t;

typedef struct HInnerQueue
{
    HNode_t *head;
    HNode_t *tail;
    pthread_mutex_t mutex;
} HInnerQueue_t;

typedef struct HQueue
{
    HInnerQueue_t level[LEVELS];
} HQueue_t;
