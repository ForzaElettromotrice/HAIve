//
// Created by f3m on 24/04/25.
//

#include "hashmap.h"


int initHashmap(Hashmap_t **hashmap)
{
    *hashmap = malloc(sizeof(Hashmap_t));
    if (!*hashmap)
    {
        E_Print("malloc: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    (*hashmap)->keys = calloc(512, sizeof(char *));
    if (!(*hashmap)->keys)
    {
        free(*hashmap);
        E_Print("malloc: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    (*hashmap)->values = calloc(512, sizeof(char *));
    if (!(*hashmap)->values)
    {
        free(*hashmap);
        free((*hashmap)->keys);
        E_Print("malloc: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
void freeHashmap(Hashmap_t *hashmap)
{
    for (int i = 0; i < 512; ++i)
    {
        if (hashmap->keys[i] != NULL)
            free(hashmap->keys[i]);
        if (hashmap->values[i] != NULL)
            free(hashmap->values[i]);
    }
    free(hashmap->keys);
    free(hashmap->values);
    free(hashmap);
}

int setByKey(const char *key, const void *val, const size_t size, const Hashmap_t *hashmap)
{
    int_fast16_t idx = (int_fast16_t) XXH3_64bits(key, strlen(key)) & 512;

    while (true)
    {
        if (hashmap->keys[idx] == NULL)
            break;
        idx = idx + 1 & 512;
    }

    hashmap->keys[idx] = strdup(key);
    hashmap->values[idx] = malloc(size);
    if (!hashmap->values[idx])
    {
        E_Print("malloc: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    memcpy(hashmap->values[idx], val, size);
    return EXIT_SUCCESS;
}
void *getByKey(const char *key, const Hashmap_t *hashmap)
{
    int_fast16_t idx = (int_fast16_t) XXH3_64bits(key, strlen(key)) & 512;

    while (true)
    {
        if (strcmp(hashmap->keys[idx], key) == 0)
            break;
        idx = idx + 1 & 512;
    }

    return hashmap->values[idx];
}

void removeKey(const char *key, const Hashmap_t *hashmap)
{
    int_fast16_t idx = (int_fast16_t) XXH3_64bits(key, strlen(key)) & 512;

    while (true)
    {
        if (strcmp(hashmap->keys[idx], key) == 0)
            break;
        idx = idx + 1 & 512;
    }

    free(hashmap->keys[idx]);
    free(hashmap->values[idx]);
    hashmap->keys[idx] = NULL;
    hashmap->values[idx] = NULL;
}
