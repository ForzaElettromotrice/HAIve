//
// Created by minga on 06/01/2025.
//

#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "engine.h"

#ifdef WIN32
size_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

int mainLoop(Board_t *board);