//
// Created by f3m on 23/06/25.
//

#include <pthread.h>
#include <stdlib.h>
#include <logger.h>

#include "tree.h"

#include "utils.h"

pthread_t thread;
Node_t *root;

void *negamax(void *args)
{








    return EXIT_SUCCESS;
}

int initTree()
{
    root = calloc(1, sizeof(Node_t));
    initContext(root->context);

    const int result = pthread_create(&thread, NULL, negamax, NULL);
    if (result)
    {
        logE(stderr, "pthread_create: %d\n", result);
        return EXIT_FAILURE;
    }

    //TODO: free tutto l'albero
    return EXIT_SUCCESS;
}

