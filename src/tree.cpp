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
HInnerQueue_t *batchQueue;
Hashmap_t hashtable;

pthread_t threads[THREADS_NUM];
pthread_t dfsThread;
pthread_t batchThread;
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
    node->bestChoice = nullptr;
}
void freeNode(Node_t *node)
{
    free(node->moves);
    cleanContext(&node->context);
    free(node->childs);
    free(node);
}


void dfs(Node_t *node, const uint8_t depth)
{
    if (node->score == -2 || depth == LEVELS)
        return;

    double score = -2;
    Node_t *bestChild = nullptr;
    for (uint_fast8_t i = 0; i < node->cCount; ++i)
    {
        dfs(&node->childs[i], depth + 1);

        const double cScore = -node->childs[i].score;
        if (cScore == 2)
            continue;

        if (cScore > score)
        {
            score = cScore;
            bestChild = &node->childs[i];
        }
    }
    node->score = score;
    node->bestChoice = bestChild;
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

                //Iscrivo il figlio al calcolo del valore
                simplehpush(batchQueue, child);
                hpush(workQueue, level + 1, child);
            }
        }
    }
    return nullptr;
}
void *evaluateTree(void *args)
{
    while (!stop)
    {
        dfs(root, 0);
    }
    return nullptr;
}
void *evaluateNodes(void *args)
{
    BatchContext bContext;
    timespec t1, t2;
    while (!stop)
    {
        bContext.count = 0;
        clock_gettime(CLOCK_MONOTONIC, &t1);
        while (bContext.count < BATCH_NUM)
        {
            clock_gettime(CLOCK_MONOTONIC, &t2);
            if (t2.tv_sec - t1.tv_sec + (t2.tv_nsec - t1.tv_nsec) * 1e-9 > MIN_T)
                break;

            const auto node = static_cast<Node_t *>(simplehpop(batchQueue));
            if (node == nullptr)
                continue;
            bContext.nodes[bContext.count++] = node;
        }

        //TODO: chiamare il forward della rete
    }
    return nullptr;
}


int initTree()
{
    //Init rete
    model = HiveCNNEnhanced(MODEL_PATH);
    model->load_model();

    //Init work queue
    workQueue = initHQueue();

    //Init batch queue
    batchQueue = initSimpleHQueue();

    //Init Hashmap
    initHashmap(&hashtable, 16384);


    //Init root
    const auto node = static_cast<Node_t *>(malloc(sizeof(Node_t)));
    Context_t context;
    initContext(&context);
    initNode(node, nullptr, &context);
    node->score = 0; //Pareggio, nessuno ha mosso
    hpush(workQueue, 0, node);

    //Init threads
    stop = false;
    for (int i = 0; i < THREADS_NUM; ++i)
    {
        pthread_create(&threads[i], nullptr, expandNode, nullptr);
    }

    //Init dfs
    pthread_create(&dfsThread, nullptr, evaluateTree, nullptr);

    //Init evaluation nodes
    pthread_create(&batchThread, nullptr, evaluateNodes, nullptr);

    return EXIT_SUCCESS;
}
void cleanTree()
{
    //Clean threads
    stop = true;
    for (const unsigned long thread: threads)
    {
        pthread_join(thread, nullptr);
    }
    //Clean dfs
    pthread_join(dfsThread, nullptr);

    //Clean evaluation nodes
    pthread_join(batchThread, nullptr);

    //Clean Hashmap
    freeHashmap(&hashtable);

    //Clean work queue
    cleanHQueue(workQueue, true);

    //Clean batch queue
    cleanSimpleHQueue(batchQueue, true);


    //TODO: Clean rete
}

