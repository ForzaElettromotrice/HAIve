//
// Created by f3m on 23/06/25.
//

#include <pthread.h>
#include <stdlib.h>


#include <logger.h>
#include <hashmap.h>

#include "hivecnn.hpp"
#include "tree.hpp"

#include "heuristics.hpp"
#include "utils.h"

pthread_t threads[15];
uint8_t tCount;
bool kill;

Node_t *root;
Hashmap_t *hashtable;
HiveCNNEnhanced model;


void initNode(Node_t *node, Piece_t *move, const Context_t *context)
{
    node->move = move;
    copyContext(context, node->context);
    addOurMove(node->context, move);
    node->childs = static_cast<Node_t *>(malloc(150 * sizeof(Node_t)));
    node->score = -2;
    node->cCount = 0;
    node->isComplete = false;
    node->hash = hashAll(node->context->board, node->context->idToPos);
}
void evaluateNode(Node_t *node);


void updateHashtable(const Node_t *node, const uint8_t depthCNN)
{
    //TODO: ragionare meglio su come e quando aggiornare i valori
    //TODO: lock
    const uint64_t hash = hashAll(node->context->board, node->context->idToPos);
    const auto result = static_cast<HashValue *>(getByHash(hash, hashtable));
    if (result == nullptr || result->depthCNN >= depthCNN)
    {
        const HashValue_t val = {node->score, true, depthCNN};
        setByHash(hash, &val, sizeof(HashValue_t), hashtable);
    }
}


void *negamax(void *args)
{
    const ThreadArgs_t *tArgs = static_cast<ThreadArgs_t *>(args);
    const uint8_t depth = tArgs->depth;
    Node_t *node = tArgs->node;

    if (depth == 5)
    {
        node->score = model->forward(node->context).item<float>();
        updateHashtable(node, 5);
        return nullptr;
    }

    //euristica su me stesso
    node->score = mzingaHeuristic(node->context);
    updateHashtable(node, 0);

    pthread_t childs[15];
    uint8_t cCount = 0;
    for (uint_fast8_t i = 0; i < 15; ++i)
    {
        for (uint_fast8_t j = 0; node->moves[i][j].id != NULLPIECE; ++j)
        {
            //TODO: riciclo dei nodi
            Node_t child = node->childs[node->cCount++];
            initNode(&child, &node->moves[i][j], node->context);

            const HashValue_t *result = static_cast<HashValue_t *>(getByHash(child.hash, hashtable));
            if (result == nullptr || (!result->isInTree && result->depthCNN > depth))
            {
                auto cArgs = (ThreadArgs_t){static_cast<uint8_t>(depth + 1), &child};
                if (tCount <= 15)
                {
                    //TODO: lock
                    pthread_create(&threads[tCount], nullptr, negamax, &cArgs);
                    childs[cCount++] = threads[tCount++];
                    continue;
                }
                negamax(&cArgs);

                if (child.score * -1 > node->score)
                {
                    node->score = child.score;
                    updateHashtable(node, 0);
                }
            }
        }
    }

    for (int i = 0; i < cCount; ++i)
        pthread_join(childs[i], nullptr);

    for (int i = 0; i < node->cCount; ++i)
    {
        if (node->childs[i].score * -1 > node->score)
        {
            node->score = node->childs[i].score * -1;
        }
    }
    updateHashtable(node, depth);

    return nullptr;
}

int initTree()
{
    root = static_cast<Node_t *>(calloc(1, sizeof(Node_t)));
    initContext(root->context);

    initHashmap(hashtable, 8192);
    model = HiveCNNEnhanced(MODEL_PATH);
    model->load_model();

    //TODO: inizializza rete


    const int result = pthread_create(&threads[tCount++], nullptr, negamax, root);
    if (result)
    {
        logE(stderr, "pthread_create: %d\n", result);
        return EXIT_FAILURE;
    }

    //TODO: free tutto l'albero
    return EXIT_SUCCESS;
}


int getBestChild()
{
    //for sui figli della radice e prende il max
}
