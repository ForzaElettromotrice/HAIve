//
// Created by f3m on 03/07/25.
//

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

#define LEVELS 5

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

bool isHEmpty(const HQueue_t *queue, uint_fast8_t level);
bool isSimpleHEmpty(const HInnerQueue_t *queue);


HQueue_t *initHQueue();
void cleanHQueue(HQueue_t *queue, bool freeVals);

void hpush(HQueue_t *queue, uint_fast8_t level, void *val);
void *hpop(HQueue_t *queue, uint_fast8_t *level);

void swapPriority(HQueue_t *queue);


HInnerQueue_t *initSimpleHQueue();
void cleanSimpleHQueue(HInnerQueue_t *queue, bool freeVals);

void simplehpush(HInnerQueue_t *queue, void *val);
void *simplehpop(HInnerQueue_t *queue);

#ifdef __cplusplus
}
#endif

