//
// Created by f3m on 28/03/25.
//

#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "logger.h"
#include <assert.h>
#include <enums.h>
#include <stdlib.h>
#include <string.h>
#include "xxhash.h"

typedef struct GameType {
    bool ladybug;
    bool pillbug;
    bool mosquito;
} GameType;

enum GameStatus {
    WHITE_WON,
    BLACK_WON,
    DRAW,
    IN_PROGRESS,
    NOT_STARTED,
};