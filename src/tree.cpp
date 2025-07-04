//
// Created by f3m on 23/06/25.
//

#include <pthread.h>
#include <hashmap.h>

#include "hivecnn.hpp"
#include "tree.hpp"
#include "HQueue.h"
#include "utils.h"


HiveCNNEnhanced model;
HQueue_t *workQueue;
Hashmap_t hashtable;

pthread_t threads[THREADS_NUM];
volatile bool stop;

Node_t *root;

bool isExpandable(const Node_t *node)
{
    const HashValue_t *val = static_cast<HashValue_t *>(getByHash(node->hash, &hashtable));

    //TODO: criteri di espansione

    return true;
}
void initNode(Node_t *node, Piece_t *move, const Context_t *context)
{
    if (node == nullptr)
        return;

    node->move = move;
    copyContext(context, &node->context);
    addOurMove(&node->context, move);
    node->childs = static_cast<Node_t *>(malloc(300 * sizeof(Node_t)));
    node->score = -2;
    node->cCount = 0;
    node->hash = hashAll(node->context.board, node->context.idToPos);
}
void freeNode(Node_t *node)
{
    free(node->moves);
    cleanContext(&node->context);
    free(node->childs);
    free(node);
}

void *expandNode(void *args)
{
    while (!stop)
    {
        uint_fast8_t level;
        Node_t *node;
        do
        {
            node = static_cast<Node_t *>(hpop(workQueue, &level));
        } while (node == nullptr);

        if (level == 10)
        {
            hpush(workQueue, level, node);
            continue;
        }

        //Valuto il nodo corrente
        node->score = model->forward(&node->context).item<float>();


        for (int i = 0; i < 15; ++i)
        {
            for (int j = 0; node->moves[MMtA(i, j)].id != NULLPIECE; ++j)
            {
                //TODO: riciclo nodi
                const auto child = static_cast<Node_t *>(malloc(sizeof(Node_t)));
                initNode(child, &node->moves[MMtA(i, j)], &node->context);


                if (!isExpandable(child))
                {
                    //TODO: ricicla
                    freeNode(child);
                    continue;
                }

                getMoves(&child->context, &child->moves);
                hpush(workQueue, level + 1, child);
            }
        }
    }
    return nullptr;
}
void *evaluateTree(void *args)
{
}

int initTree()
{
    //Init rete
    model = HiveCNNEnhanced(MODEL_PATH);
    model->load_model();

    //Init work queue
    workQueue = initHQueue();

    //Init Hashmap
    initHashmap(&hashtable, 16384);


    //Init root
    const auto node = static_cast<Node_t *>(malloc(sizeof(Node_t)));
    const auto context = static_cast<Context_t *>(malloc(sizeof(Context_t)));
    initContext(context);
    initNode(node, nullptr, context);
    hpush(workQueue, 0, node);

    //Init threads
    stop = false;
    for (int i = 0; i < THREADS_NUM; ++i)
    {
        pthread_create(&threads[i], nullptr, expandNode, nullptr);
    }

    return EXIT_SUCCESS;
}
void cleanTree()
{
    //Clean threads
    stop = true;
    for (int i = 0; i < THREADS_NUM; ++i)
    {
        pthread_join(threads[i], nullptr);
    }

    //Clean Hashmap
    freeHashmap(&hashtable);

    //Clean work queue
    cleanHQueue(workQueue, true);

    //TODO: Clean rete
}


int getBestChild()
{
    //for sui figli della radice e prende il max
}
