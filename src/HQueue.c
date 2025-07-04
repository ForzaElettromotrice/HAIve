//
// Created by f3m on 03/07/25.
//

#include <HQueue.h>
#include <stdlib.h>

#include "logger.h"

bool isHEmpty(const HQueue_t *queue, const int_fast8_t level)
{
    if (level >= LEVELS)
    {
        logE(stderr, "Level %d is too high. Max %d levels\n", level, LEVELS);
        return true;
    }
    return queue->level[level].head == NULL;
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
void cleanHQueue(HQueue_t *queue, bool freeVals)
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

void hpush(HQueue_t *queue, const int_fast8_t level, void *val)
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
void *hpop(HQueue_t *queue)
{
    int_fast8_t level = 0;
    while (true)
    {
        if (level == LEVELS)
            break;
        pthread_mutex_lock(&queue->level[level].mutex);
        if (!isHEmpty(queue, level))
            break;
        pthread_mutex_unlock(&queue->level[level].mutex);
        level++;
    }

    if (level >= LEVELS)
    {
        logE(stderr, "Level %d is too high. Max %d levels\n", level, LEVELS);
        return NULL;
    }


    HNode_t *node = queue->level[level].head;
    void *val = node->val;

    if (queue->level[level].head == queue->level[level].tail)
    {
        queue->level[level].head = NULL;
        queue->level[level].tail = NULL;
    } else
        queue->level[level].head = node->next;
    pthread_mutex_unlock(&queue->level[level].mutex);
    free(node);
    return val;
}
