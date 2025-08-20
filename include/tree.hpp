//
// Created by f3m on 14/08/25.
//

#pragma once

#include <enums.h>
#include <cstdint>
#include <moves.h>
#include <phmap.h>

#define THREADS_NUM 1
#define BATCH_NUM 64

//TODO: da rivedere
#define MIN_T 1.0
//TODO: da rivedere
#define KK 400
//TODO: da rivedere
#define MIN_MEM 5000000L


typedef struct Node
{
    uint64_t id;
    uint64_t ccRootsCount; //creation change roots count
    bool isInWorkQueue;
    uint16_t cCount;

    Piece_t move;
    Piece_t moves[MOVES_SIZE];
    HAIveContext_t context;

    Node *bestChild;
    Node *childs[MOVES_SIZE];

    double score;

    uint64_t hash;
} Node_t;


typedef struct BatchContext
{
    Node_t *nodes[BATCH_NUM];
    uint8_t count;
    double result[BATCH_NUM];
} BatchContext_t;

struct IdentityHash
{
    size_t operator()(const uint64_t key) const { return key; }
};

using Map = phmap::parallel_flat_hash_map<
    uint64_t, //tipo chiave
    double, //tipo valore
    IdentityHash, //hash che usa
    std::equal_to<>, //comparatore fra chiavi
    std::allocator<std::pair<uint64_t, double> >, //allocatore
    6, //sub-map (2^6)
    std::mutex //per la concorrenza
>;

int initTree();
void cleanTree();

const Node_t *getBestChild();
void adversaryMove(char *mzingaMove);
