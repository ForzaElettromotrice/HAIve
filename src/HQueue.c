//
// Created by f3m on 03/07/25.
//

#include <HQueue.h>
#include <stdlib.h>

#include "logger.h"

bool isHEmpty(const HQueue_t *queue, const uint_fast8_t level)
{
    if (level >= LEVELS)
    {
        logE(stderr, "Level %d is too high. Max %d levels\n", level, LEVELS);
        return true;
    }
    return queue->level[level].head == NULL;
}
bool isSimpleHEmpty(const HInnerQueue_t *queue)
{
    return queue->head == NULL;
}

HQueue_t *initHQueue()
{
    HQueue_t *queue = calloc(1, sizeof(HQueue_t));

    for (int i = 0; i < LEVELS; ++i)
    {
        pthread_mutex_init(&queue->level[i].mutex, NULL);
    }

    return queue;
}
void cleanHQueue(HQueue_t *queue, const bool freeVals)
{
    for (int i = 0; i < LEVELS; ++i)
    {
        pthread_mutex_destroy(&queue->level[i].mutex);
        HNode_t *start = queue->level[i].head;
        while (start != NULL)
        {
            HNode_t *next = start->next;
            if (freeVals)
                free(start->val);
            free(start);
            start = next;
        }
    }
}

void hpush(HQueue_t *queue, const uint_fast8_t level, void *val)
{
    if (level >= LEVELS)
    {
        logE(stderr, "Level %d is too high. Max %d levels\n", level, LEVELS);
        return;
    }

    HNode_t *node = malloc(sizeof(HNode_t));
    node->val = val;
    node->next = NULL;
    pthread_mutex_lock(&queue->level[level].mutex);

    if (isHEmpty(queue, level))
        queue->level[level].head = node;
    else
        queue->level[level].tail->next = node;
    queue->level[level].tail = node;

    pthread_mutex_unlock(&queue->level[level].mutex);
}
void *hpop(HQueue_t *queue, uint_fast8_t *level)
{
    *level = 0;
    while (true)
    {
        if (*level == LEVELS)
            break;
        pthread_mutex_lock(&queue->level[*level].mutex);
        if (!isHEmpty(queue, *level))
            break;
        pthread_mutex_unlock(&queue->level[*level].mutex);
        (*level)++;
    }

    if (*level >= LEVELS)
        return NULL;


    HNode_t *node = queue->level[*level].head;
    void *val = node->val;

    if (queue->level[*level].head == queue->level[*level].tail)
    {
        queue->level[*level].head = NULL;
        queue->level[*level].tail = NULL;
    } else
        queue->level[*level].head = node->next;
    pthread_mutex_unlock(&queue->level[*level].mutex);
    free(node);
    return val;
}

void swapPriority(HQueue_t *queue)
{
    for (int_fast8_t i = 0; i < LEVELS; ++i)
    {
        pthread_mutex_lock(&queue->level[i].mutex);
    }

    HNode_t *start = queue->level[0].head;
    while (start != NULL)
    {
        HNode_t *next = start->next;
        free(start);
        start = next;
    }

    for (int_fast8_t i = 0; i < LEVELS - 1; ++i)
    {
        queue->level[i].head = queue->level[i + 1].head;
        queue->level[i].tail = queue->level[i + 1].tail;
    }
    queue->level[LEVELS - 1].head = NULL;
    queue->level[LEVELS - 1].tail = NULL;


    for (int_fast8_t i = 0; i < LEVELS; ++i)
    {
        pthread_mutex_unlock(&queue->level[i].mutex);
    }
}

//TODO: se una coda è bloccata, prova a usarne un altra


HInnerQueue_t *initSimpleHQueue()
{
    HInnerQueue_t *queue = calloc(1, sizeof(HInnerQueue_t));

    pthread_mutex_init(&queue->mutex, NULL);

    return queue;
}
void cleanSimpleHQueue(HInnerQueue_t *queue, const bool freeVals)
{
    pthread_mutex_destroy(&queue->mutex);
    HNode_t *start = queue->head;
    while (start != NULL)
    {
        HNode_t *next = start->next;
        if (freeVals)
            free(start->val);
        free(start);
        start = next;
    }
}
void simplehpush(HInnerQueue_t *queue, void *val)
{
    HNode_t *node = malloc(sizeof(HNode_t));
    node->val = val;
    node->next = NULL;
    pthread_mutex_lock(&queue->mutex);

    if (isSimpleHEmpty(queue))
        queue->head = node;
    else
        queue->tail->next = node;
    queue->tail = node;

    pthread_mutex_unlock(&queue->mutex);
}
void *simplehpop(HInnerQueue_t *queue)
{
    pthread_mutex_lock(&queue->mutex);
    HNode_t *node = queue->head;

    if (node == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    void *val = node->val;

    if (queue->head == queue->tail)
    {
        queue->head = NULL;
        queue->tail = NULL;
    } else
        queue->head = node->next;

    pthread_mutex_unlock(&queue->mutex);
    free(node);
    return val;
}
