//
// Created by minga on 06/01/2025.
//

#pragma once

typedef enum Pieces
{
    NULLPIECE = -1,
    B_QUEEN,
    B_PILLBUG,
    B_LADYBUG,
    B_MOSQUITO,
    B_ANT_1,
    B_ANT_2,
    B_ANT_3,
    B_GRASSHOPPER_1,
    B_GRASSHOPPER_2,
    B_GRASSHOPPER_3,
    B_BEETLE_1,
    B_BEETLE_2,
    B_SPIDER_1,
    B_SPIDER_2,
    W_QUEEN,
    W_PILLBUG,
    W_LADYBUG,
    W_MOSQUITO,
    W_ANT_1,
    W_ANT_2,
    W_ANT_3,
    W_GRASSHOPPER_1,
    W_GRASSHOPPER_2,
    W_GRASSHOPPER_3,
    W_BEETLE_1,
    W_BEETLE_2,
    W_SPIDER_1,
    W_SPIDER_2,
} Pieces_t;

typedef enum Colors
{
    NULLCOLOR = 0,
    WHITE = 1,
    BLACK = -1
} Colors_t;
