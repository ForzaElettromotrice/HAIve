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
#include <hivecnn.hpp>

#include "heuristics.hpp"


HQueue_t *workQueue;
HInnerQueue_t *batchQueue;
HInnerQueue_t *garbageQueue;

Map hashmap;
HiveNet *model;
static HiveCNNEnhanced modelBase;

pthread_t threads[THREADS_NUM];
pthread_t dfsThread;
pthread_t batchThread;
pthread_t chrootThread;

volatile bool stop;
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
    const uint64_t delta = changeRootsCount - node->ccRootsCount;
    return node->id / ipow(KK, delta);
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

void initRoot()
{
    root = static_cast<Node_t *>(malloc(sizeof(Node_t)));
    HAIveContext_t context;
    initHAIveContext(&context);
    initHAIveContext(&root->context);

    root->id = 1;
    root->ccRootsCount = 0;
    root->isInWorkQueue = true;

    root->move = nullptr;
    memset(root->moves, 0xff, sizeof(root->moves));
    copyHAIveContext(&context, &root->context);

    root->bestChild = nullptr;
    root->cCount = 0;

    root->score = 0;

    root->hash = hashAll(root->context.board, root->context.idToPos, root->context.curColor);

    getMoves(&root->context, root->moves);
    hpush(workQueue, 0, root);
}
void initNode(Node_t *node, const Node_t *father, const uint64_t id, const uint8_t relativeDepth, const Piece_t *move)
{
    node->id = normalizeId(father) + (id + 1) * ipow(KK, relativeDepth);
    node->ccRootsCount = changeRootsCount;
    node->isInWorkQueue = false;

    node->move = move;
    memset(node->moves, 0xff, sizeof(node->moves));
    initHAIveContext(&node->context);
    copyHAIveContext(&father->context, &node->context);
    addHAIveMove(&node->context, move);

    node->bestChild = nullptr;
    node->cCount = 0;

    node->score = -2;

    node->hash = hashAll(node->context.board, node->context.idToPos, node->context.curColor);
}
void resetNode(Node_t *node, const Node_t *father, const uint64_t id, const uint8_t relativeDepth, const Piece_t *move)
{
    node->id = normalizeId(father) + (id + 1) * (ipow(KK, relativeDepth));
    node->ccRootsCount = changeRootsCount;
    node->isInWorkQueue = false;
    node->move = move;

    memset(node->moves, 0xff, sizeof(node->moves));
    copyHAIveContext(&father->context, &node->context);
    addHAIveMove(&node->context, move);

    node->bestChild = nullptr;
    node->cCount = 0;

    node->score = -2;

    node->hash = hashAll(node->context.board, node->context.idToPos, node->context.curColor);
}
Node_t *getNewNode(const Node_t *father, const uint64_t id, const uint8_t relativeDepth, const Piece_t *move)
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

void dfs(Node_t *node, const uint8_t depth)
{
    if (node->score == -2 || depth == LEVELS || stop || pause)
        return;

    double maxScore = -2;
    Node_t *bestChild = nullptr;

    //Prendiamo un valore approssimativo prima di andare in ricorsione per evitare di non fare in tempo
    for (uint_fast16_t i = 0; i < node->cCount; ++i)
    {
        const double cScore = -node->childs[i]->score;
        if (cScore == 2 || cScore <= maxScore)
            continue;

        maxScore = cScore;
        bestChild = node->childs[i];
    }

    node->score = maxScore;
    node->bestChild = bestChild;
    if (const auto it = hashmap.find(node->hash); it != hashmap.end())
        it->second.score = maxScore;

    //andiamo in ricorsione
    for (uint_fast16_t i = 0; i < node->cCount; ++i)
    {
        if (stop || pause)
            break;

        dfs(node->childs[i], depth + 1);

        const double cScore = -node->childs[i]->score;
        if (cScore == 2 || cScore <= node->score)
            continue;

        //Aggiorniamo il valore dopo la ricorsione se troviamo un nodo migliore
        node->score = cScore;
        node->bestChild = node->childs[i];
        if (const auto it = hashmap.find(node->hash); it != hashmap.end())
            it->second.score = cScore;
    }
}
void garbageCollector(Node_t *node, const bool first)
{
    if (!first && (node->isInWorkQueue || isInTree(node)))
        return;

    for (uint16_t i = 0; i < node->cCount; ++i)
    {
        garbageCollector(node->childs[i], false);
    }
    simplehpush(garbageQueue, node);
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

        node->isInWorkQueue = false;

        if (!isInTree(node))
        {
            simplehpush(garbageQueue, node);
            continue;
        }

        //FIXME: se cambiamo il modo in cui vengono messi in priorità questo non serve più
        //Se al livello massimo non possiamo espandere di più
        if (level == LEVELS - 1)
        {
            node->isInWorkQueue = true;
            hpush(workQueue, level, node);
            continue;
        }

        bool passBool = true;
        for (int_fast16_t i = 0; node->moves[i].id != NULLPIECE; ++i)
        {
            passBool = false;
            auto *child = getNewNode(node, i, level + 1, &node->moves[i]);
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
                val.score = mzingaHeuristic(&child->context);
                memcpy(&val.moves, child->moves, sizeof(child->moves));
                hashmap[child->hash] = val;

                simplehpush(batchQueue, child);
            }

            child->isInWorkQueue = true;
            insertIntoWorkQueue(child, level + 1);
        }

        if (passBool)
        {
            //FIXME: se cambiamo il modo in cui i nodi sono messi nella queue allora tocca passa il relative depth in modo diverso
            node->moves[0] = pass;
            auto *child = getNewNode(node, 0, level + 1, &node->moves[0]);
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
                val.score = mzingaHeuristic(&child->context);
                memcpy(&val.moves, child->moves, sizeof(child->moves));
                hashmap[child->hash] = val;

                simplehpush(batchQueue, child);
            }

            child->isInWorkQueue = true;
            insertIntoWorkQueue(child, level + 1);
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
    BatchContext_t bContext = {};
    timespec t1 = {};
    timespec t2 = {};
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
        for (uint_fast8_t i = 0; i < bContext.count; ++i)
        {
            Node_t *node = bContext.nodes[i];
            node->score = bContext.result[i];

            hashmap.find(node->hash)->second.score = node->score;
        }
    }

    return nullptr;
}
void *changeRoot(void *args)
{
    const auto newRoot = static_cast<Node_t *>(args);

    swapPriority(workQueue);
    Node_t *oldRoot = root;
    root = newRoot;
    changeRootsCount++;

    pause = false;
    pthread_barrier_wait(&pauseBarrier);

    garbageCollector(oldRoot, true);
    return nullptr;
}

int initTree()
{
    //Init rete
    modelBase = HiveCNNEnhanced(MODEL_PATH);
    model = modelBase.get();
    model->load_model();

    //Init queues
    workQueue = initHQueue();
    batchQueue = initSimpleHQueue();
    garbageQueue = initSimpleHQueue();
    //Init changeRoots
    changeRootsCount = 0;

    //Init root
    initRoot();

    //Init barrier
    pthread_barrier_init(&pauseBarrier, nullptr, THREADS_NUM + 3); //numero di thread + dfs + batch + chroot

    //Init threads
    stop = false;
    pause = false;
    for (unsigned long &thread: threads)
    {
        pthread_create(&thread, nullptr, expandNode, nullptr);
    }
    pthread_create(&dfsThread, nullptr, evaluateTree, nullptr);
    pthread_create(&batchThread, nullptr, evaluateNodes, nullptr);

    chrootThread = 0;

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
    pthread_join(dfsThread, nullptr);
    pthread_join(batchThread, nullptr);
    if (chrootThread != 0)
        pthread_join(chrootThread, nullptr);

    //Clean barrier
    pthread_barrier_destroy(&pauseBarrier);

    //Clean hashmap
    hashmap.clear();

    //Clean queues
    cleanHQueue(workQueue, true);
    cleanSimpleHQueue(batchQueue, true);
    cleanSimpleHQueue(garbageQueue, true);

    //TODO: clean rete
}

const Node_t *getBestChild()
{
    pause = true;
    pthread_barrier_wait(&pauseBarrier);
    if (root->bestChild == nullptr)
    {
        // FIXME: Momentaneo
        return root->childs[0];
        logE(stderr, "Best child is null\nProbably not enough time to perform a dfs");
        return nullptr;
    }
    pthread_create(&chrootThread, nullptr, changeRoot, root->bestChild);

    return root->bestChild;
}
void adversaryMove(char *mzingaMove)
{
    const Piece_t move = parseMove(root->context.idToPos, mzingaMove);

    pause = true;
    pthread_barrier_wait(&pauseBarrier);

    Node_t *newRoot = nullptr;
    for (uint_fast16_t i = 0; i < root->cCount; ++i)
    {
        if (equalsPiece(&move, root->childs[i]->move))
        {
            newRoot = root->childs[i];
            break;
        }
    }

    if (newRoot == nullptr)
    {
        logE(stderr, "Non dovremmo essere qui\n");
        return;
    }

    pthread_create(&chrootThread, nullptr, changeRoot, newRoot);
}
