//
// Created by f3m on 24/04/25.
//

#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <xxhash.h>
#include <logger.h>


typedef struct Hashmap
{
    char **keys;
    void **values;
} Hashmap_t;

int initHashmap(Hashmap_t **hashmap);
void freeHashmap(Hashmap_t *hashmap);

int setByKey(const char *key, const void *val, size_t size, const Hashmap_t *hashmap);
void *getByKey(const char *key, const Hashmap_t *hashmap);

void removeKey(const char *key, const Hashmap_t *hashmap);
