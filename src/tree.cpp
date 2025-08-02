//
// Created by f3m on 23/06/25.
//

#include <pthread.h>
#include <hashmap.h>

#include "hivecnn.hpp"
#include "tree.hpp"
#include "HQueue.h"
#include "utils.h"


HiveNet *model;
HQueue_t *workQueue;
HInnerQueue_t *batchQueue;
HInnerQueue_t *garbageQueue;
Hashmap_t hashtable;
pthread_mutex_t hLock;

pthread_t threads[THREADS_NUM];
pthread_t dfsThread;
pthread_t batchThread;
pthread_t chrootThread;
volatile bool stop;
pthread_barrier_t b;
volatile bool pause;
uint64_t changeRoots;

Node_t *root;

void alertK(const int16_t cCount)
{
    if (cCount > 140)
    {
        logE(stderr, "cCount > 140! cCount = %d\n", cCount);
        return;
    }
    if (cCount > 120)
    {
        logE(stderr, "cCount > 120! cCount = %d\n", cCount);
        return;
    }
    if (cCount > 100)
    {
        logE(stderr, "cCount > 100! cCount = %d\n", cCount);
        return;
    }
    if (cCount > 80)
    {
        logE(stderr, "cCount > 80! cCount = %d\n", cCount);
        return;
    }
    if (cCount > 70)
    {
        logE(stderr, "idx > 70! cCount = %d\n", cCount);
    }
}

uint64_t normalizeId(const Node_t *father)
{
    return static_cast<uint64_t>(father->id / pow(K, changeRoots - father->cRoots));
}
bool isInTree(const Node_t *node)
{
    // FIXME: Va bene sta roba? Altrimenti al primo ciclo crasha perchè poi prova ad accedere a father->id
    if (root == nullptr)
        return true;
    return node->id % K == normalizeId(root);
}
void checkPause()
{
    if (pause)
    {
        pthread_barrier_wait(&b);
        pthread_barrier_wait(&b);
    }
}
bool alreadySeen(Node_t *node)
{
    if (node == nullptr)
    {
        logE(stderr, "Node non dovrebbe essere nullptr\n");
        return false;
    }
    const HashValue_t *val = static_cast<HashValue_t *>(getByHash(node->hash, &hashtable));
    if (val == nullptr)
        return false;

    node->score = val->score;
    node->moves = val->moves;

    return true;
}
void initNode(Node_t *node, const Piece_t *move, const HAIveContext_t *context, Node_t *father, uint8_t level)
{
    if (node == nullptr)
        return;

    node->move = move;
    initHAIveContext(&node->context);
    copyHAIveContext(context, &node->context);
    addHAIveMove(&node->context, move);
    node->childs = static_cast<Node_t **>(malloc(300 * sizeof(Node_t *)));
    node->score = -2;
    node->cCount = 0;
    node->hash = hashAll(node->context.board, node->context.idToPos, node->context.curColor);
    node->bestChoice = nullptr;
    node->cRoots = changeRoots;
    if (father == nullptr)
        node->id = 1;
    else
        node->id = normalizeId(father) + (father->cCount + 1) * pow(K, level + 1);
}
void freeNode(Node_t *node)
{
    free(node->moves);
    cleanHAIveContext(&node->context);
    free(node->childs);
    free(node);
}
Node_t *getNewNode(const Piece_t *move, const HAIveContext_t *context, Node_t *father, int16_t level)
{
    auto *node = static_cast<Node_t *>(simplehpop(garbageQueue));
    if (node == nullptr)
    {
        node = static_cast<Node_t *>(malloc(sizeof(Node_t)));
        initNode(node, move, context, father, level);
        return node;
    }

    node->move = move;
    free(node->moves);
    node->moves = nullptr;
    node->score = -2;
    copyHAIveContext(context, &node->context);
    addHAIveMove(&node->context, move);
    node->cCount = 0;
    node->bestChoice = nullptr;
    node->hash = hashAll(node->context.board, node->context.idToPos, node->context.curColor);
    node->cRoots = changeRoots;
    node->id = normalizeId(father) + (father->cCount + 1) * pow(K, level + 1);
    return node;
}


void dfs(Node_t *node, const uint8_t depth)
{
    if (node->score == -2 || depth == LEVELS || stop || pause)
        return;

    double score = -2;
    Node_t *bestChild = nullptr;
    for (int16_t i = 0; i < node->cCount; ++i)
    {
        if (stop || pause)
            break;
        dfs(node->childs[i], depth + 1);

        const double cScore = -node->childs[i]->score;
        if (cScore == 2)
            continue;

        if (cScore > score)
        {
            score = cScore;
            bestChild = node->childs[i];
        }
    }
    if (stop || pause)
        return;
    node->score = score;
    node->bestChoice = bestChild;
    pthread_mutex_lock(&hLock);
    const auto hashValue = static_cast<HashValue_t *>(getByHash(node->hash, &hashtable));
    if (hashValue != nullptr)
        hashValue->score = node->score;
    pthread_mutex_unlock(&hLock);
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

        if (level == 10)
        {
            hpush(workQueue, level, node);
            continue;
        }

        if (!isInTree(node))
        {
            simplehpush(garbageQueue, node);
            continue;
        }


        bool passBool = true;
        for (int i = 0; i < 15; ++i)
        {
            for (int j = 0; node->moves[MMtA(i, j)].id != NULLPIECE; ++j)
            {
                passBool = false;
                auto *child = getNewNode(&node->moves[MMtA(i, j)], &node->context, node, level);
                node->childs[node->cCount++] = child;
                alertK(node->cCount);
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
                    getMoves(&child->context, &child->moves);
                    HashValue_t hashValue = {-2, node->moves};
                    pthread_mutex_lock(&hLock);
                    setByHash(node->hash, &hashValue, sizeof(HashValue_t), &hashtable);
                    pthread_mutex_unlock(&hLock);
                    simplehpush(batchQueue, child);
                }

                hpush(workQueue, level + 1, child);
            }
        }
        if (passBool)
        {
            auto *child = getNewNode(&pass, &node->context, node, level);
            node->childs[node->cCount++] = child;
            alertK(node->cCount);
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
                getMoves(&child->context, &child->moves);
                HashValue_t hashValue = {-2, node->moves};
                pthread_mutex_lock(&hLock);
                setByHash(node->hash, &hashValue, sizeof(HashValue_t), &hashtable);
                pthread_mutex_unlock(&hLock);
                simplehpush(batchQueue, child);
            }

            hpush(workQueue, level + 1, child);
        }
    }
    return nullptr;
}
void *evaluateTree(void *args)
{
    while (!stop)
    {
        checkPause();
        dfs(root, 0);
    }
    return nullptr;
}
void *evaluateNodes(void *args)
{
    BatchContext bContext{};
    timespec t1{}, t2{};
    while (!stop)
    {
        bContext.count = 0;
        clock_gettime(CLOCK_MONOTONIC, &t1);
        while (bContext.count < BATCH_NUM)
        {
            checkPause();
            clock_gettime(CLOCK_MONOTONIC, &t2);
            if (static_cast<double>(t2.tv_sec - t1.tv_sec) + static_cast<double>(t2.tv_nsec - t1.tv_nsec) * 1e-9 > MIN_T)
                break;

            const auto node = static_cast<Node_t *>(simplehpop(batchQueue));
            if (node == nullptr)
                continue;
            bContext.nodes[bContext.count++] = node;
        }

        model->batchForward(&bContext);

        for (uint8_t i = 0; i < bContext.count; ++i)
        {
            Node_t *node = bContext.nodes[i];
            node->score = bContext.result[i];
            pthread_mutex_lock(&hLock);
            const auto hashValue = static_cast<HashValue_t *>(getByHash(node->hash, &hashtable));
            hashValue->score = node->score;
            pthread_mutex_unlock(&hLock);
        }
    }
    return nullptr;
}
void *changeRoot(void *args)
{
    const auto newRoot = static_cast<Node_t *>(args);

    swapPriority(workQueue);

    root = newRoot;
    changeRoots++;

    pause = false;
    pthread_barrier_wait(&b); //Libero tutti
    return nullptr;
}

int initTree()
{
    //Init rete
    HiveCNNEnhanced modelBase(MODEL_PATH);
    model = modelBase.get();
    model->load_model();

    //Init work queue
    workQueue = initHQueue();

    //Init batch queue
    batchQueue = initSimpleHQueue();

    //Init garbage queue
    garbageQueue = initSimpleHQueue();

    //Init Hashmap
    initHashmap(&hashtable, 16384);

    //Init changesRoot
    changeRoots = 0;

    //Init root
    const auto node = static_cast<Node_t *>(malloc(sizeof(Node_t)));
    HAIveContext_t context;
    initHAIveContext(&context);
    initNode(node, nullptr, &context, nullptr, 0);
    node->score = 0; //Pareggio, nessuno ha mosso
    hpush(workQueue, 0, node);

    //Init hashmap lock
    pthread_mutex_init(&hLock, nullptr);

    //Init barrier
    pthread_barrier_init(&b, nullptr, THREADS_NUM + 3); //numero di thread + dfs + batch + chroot

    //Init threads
    stop = false;
    pause = false;
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

    //Clean barrier
    pthread_barrier_destroy(&b);

    //Clean hashmap lock
    pthread_mutex_destroy(&hLock);

    //Clean Hashmap
    freeHashmap(&hashtable);

    //Clean work queue
    cleanHQueue(workQueue, true);

    //Clean batch queue
    cleanSimpleHQueue(batchQueue, false);

    //Clean garbage queue
    cleanSimpleHQueue(garbageQueue, true);


    //TODO: Clean rete
}

const Node_t *getBestChild()
{
    pause = true;
    pthread_barrier_wait(&b); //Per essere sicuri tutti siano in pausa
    pthread_create(&chrootThread, nullptr, changeRoot, root->bestChoice);

    return root->bestChoice;
}
void adversaryMove(char *mzingaMove)
{
    const Piece_t move = parseMove(root->context.idToPos, mzingaMove);

    if (equalsPiece(&move, root->move))
        return;

    pause = true;
    pthread_barrier_wait(&b); //Per essere sicuri tutti siano in pausa

    Node_t *newRoot = nullptr;

    for (int i = 0; i < root->cCount; ++i)
    {
        if (move.id == root->childs[i]->move->id && move.position.x == root->childs[i]->move->position.x && move.position.y == root->childs[i]->move->position.y && move.position.z == root->childs[i]->move->position.z)
        {
            newRoot = root->childs[i];
            break;
        }
    }

    if (newRoot == nullptr)
    {
        logE(stderr, "Non dovremmo essere qui\n");
    }

    pthread_create(&chrootThread, nullptr, changeRoot, newRoot);
}


