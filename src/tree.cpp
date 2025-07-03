//
// Created by f3m on 23/06/25.
//

#include <pthread.h>


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
    getMoves(context, &node->moves);
    node->hash = hashAll(node->context->board, node->context->idToPos);
}


int initTree()
{
    model = HiveCNNEnhanced(MODEL_PATH);
    model->load_model();


    //TODO: free tutto l'albero
    return EXIT_SUCCESS;
}


int getBestChild()
{
    //for sui figli della radice e prende il max
}
