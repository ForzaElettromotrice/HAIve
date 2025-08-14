//
// Created by f3m on 14/08/25.
//

#include <tree.hpp>
#include <HQueue.h>
#include <cmath>
#include <cstring>
#include <logger.h>
#include <cerrno>
#include <utils.h>

#include "ATen/core/interned_strings.h"

HQueue_t *workQueue;
HInnerQueue_t *garbageQueue;
HInnerQueue_t *batchQueue;

Map hashmap;


volatile
bool stop;
volatile bool pause;
pthread_barrier_t pauseBarrier;

uint64_t changeRootsCount;

Node_t *root;


void checkPause()
{
    if (pause)
    {
        pthread_barrier_wait(&pauseBarrier);
        pthread_barrier_wait(&pauseBarrier);
    }
}

uint64_t normalizeId(const Node_t *node)
{
    return node->id / static_cast<uint64_t>(pow(KK, static_cast<double>(changeRootsCount - node->ccRootsCount)));
}
bool isInTree(const Node_t *node)
{
    return node->id % KK == normalizeId(root);
}

bool alreadySeen(Node_t *node)
{
    const auto it = hashmap.find(node->hash);
    if (it == hashmap.end())
        return false;

    auto [score, moves] = it->second;
    node->score = score;
    memcpy(node->moves, moves, sizeof(moves));
    return true;
}
void insertIntoWorkQueue(Node_t *node, const uint8_t level)
{
    //TODO: qui va la logica della priorità dell'albero
    hpush(workQueue, level, node);
}

void initNode(Node_t *node, const Node_t *father, const uint64_t id, const uint8_t relativeDepth, const Piece_t *move)
{
    node->id = normalizeId(father) + (id + 1) * static_cast<uint64_t>(pow(KK, relativeDepth));
    node->ccRootsCount = changeRootsCount;

    node->move = move;
    memset(node->moves, 0xff, sizeof(node->moves));
    initHAIveContext(&node->context);
    copyHAIveContext(&father->context, &node->context);
    addHAIveMove(&node->context, move);

    node->cCount = 0;

    node->hash = hashAll(node->context.board, node->context.idToPos, node->context.curColor);
}
void resetNode(Node_t *node, const Node_t *father, const uint64_t id, const uint8_t relativeDepth, const Piece_t *move)
{
    node->id = normalizeId(father) + (id + 1) * static_cast<uint64_t>(pow(KK, relativeDepth));
    node->ccRootsCount = changeRootsCount;

    memset(node->moves, 0xff, sizeof(node->moves));
    copyHAIveContext(&father->context, &node->context);
    addHAIveMove(&node->context, move);

    node->cCount = 0;

    node->hash = hashAll(node->context.board, node->context.idToPos, node->context.curColor);
}
Node_t *getNewNode(const Node_t *father, const uint64_t id, const uint8_t relativeDepth, Piece_t *move)
{
    auto node = static_cast<Node_t *>(simplehpop(garbageQueue));
    if (node == nullptr)
    {
        node = static_cast<Node_t *>(malloc(sizeof(Node_t)));
        if (!node)
        {
            logE(stderr, "malloc: %s\n", strerror(errno));
            return nullptr;
        }

        initNode(node, father, id, relativeDepth, move);
        return node;
    }

    resetNode(node, father, id, relativeDepth, move);
    return node;
}

void *expandNode(void *args)
{
    while (!stop)
    {
        uint_fast8_t level;
        Node_t *node;
        do
        {
            checkPause();
            node = static_cast<Node_t *>(hpop(workQueue, &level));
        } while (node == nullptr);

        if (!isInTree(node))
        {
            simplehpush(garbageQueue, node);
            continue;
        }

        //Se al livello massimo non possiamo espandere di più
        if (level == LEVELS)
        {
            hpush(workQueue, level, node);
            continue;
        }

        bool passBool = true;
        for (int_fast16_t i = 0; node->moves[i].id != NULLPIECE; ++i)
        {
            passBool = false;
            auto *child = getNewNode(node, i, level, &node->moves[i]);
            if (!child)
            {
                logD(stderr, "Skipping Node %d\n", i);
                continue;
            }
            node->childs[node->cCount++] = child;

            switch (child->context.gameStatus)
            {
                case NOT_STARTED:
                    logE(stderr, "In teoria è impossibile arrivare qui\n");
                    break;
                case WHITE_WON:
                    node->score = 1;
                    continue;
                case BLACK_WON:
                    node->score = -1;
                    continue;
                case DRAW:
                    node->score = 0;
                    continue;
                case IN_PROGRESS:
                    break;
            }

            if (!alreadySeen(child))
            {
                getMoves(&child->context, child->moves);
                HashValue_t val;
                val.score = -2; //TODO: calcolo valore con la rete
                memcpy(&val.moves, child->moves, sizeof(child->moves));
                hashmap[child->hash] = val;

                simplehpush(batchQueue, child);
            }

            insertIntoWorkQueue(child, level + 1);
        }

        if (passBool)
        {
            auto *child = getNewNode(node, 0, level, &node->moves[0]);
            if (!child)
            {
                logD(stderr, "Skipping Node pass\n");
                continue;
            }
            node->childs[node->cCount++] = child;

            switch (child->context.gameStatus)
            {
                case NOT_STARTED:
                    logE(stderr, "In teoria è impossibile arrivare qui\n");
                    break;
                case WHITE_WON:
                    node->score = 1;
                    continue;
                case BLACK_WON:
                    node->score = -1;
                    continue;
                case DRAW:
                    node->score = 0;
                    continue;
                case IN_PROGRESS:
                    break;
            }

            if (!alreadySeen(child))
            {
                getMoves(&child->context, child->moves);
                HashValue_t val;
                val.score = -2; //TODO: calcolo valore con la rete
                memcpy(&val.moves, child->moves, sizeof(child->moves));
                hashmap[child->hash] = val;

                simplehpush(batchQueue, child);
            }

            insertIntoWorkQueue(child, level + 1);
        }
    }

    return nullptr;
}

int initTree() { return 0; }
void cleanTree()
{
}

const Node_t *getBestChild() { return nullptr; }
void adversaryMove(char *mzingaMove)
{
}
