//
// Created by filippo on 15/06/25.
//

#ifndef HEURISTICS_H
#define HEURISTICS_H

extern "C" {
#include "enums.h"
#include "moves.h"
}

#include "preprocessor.h"
#include <array>

#define paramSize 2*NUM_PIECES+2*NUM_PIECES

class HeuristicMetrics {
    std::array<double, 7> values_;

public:
    explicit HeuristicMetrics(const std::array<double, 7> &values) : values_(values) {
    }

    double inPlayWeight() const {
        return values_[0];
    }

    double isPinnedWeight() const {
        return values_[1];
    }

    double isCoveredWeight() const {
        return values_[2];
    }

    double noisyMoveWeight() const {
        return values_[3];
    }

    double quietMoveWeight() const {
        return values_[4];
    }

    double friendlyNeighborWeight() const {
        return values_[5];
    }

    double enemyNeighborWeight() const {
        return values_[6];
    }

    HeuristicMetrics enemy() const {
        auto new_values_ = std::array<double, 7>();
        for (uint8_t i = 0; i < 7; i++) {
            new_values_[i] = -values_[i];
        }
        return HeuristicMetrics(new_values_);
    }
};

inline HeuristicMetrics queenMetrics({
    63558.424116300164,
    -16068.975008474492,
    -10847.56765389993,
    -27548.36384306823,
    -58684.53995399401,
    6083.816305341258,
    -16777.90671222724
});

inline HeuristicMetrics spiderMetrics({
    -78668.57111290144,
    -40182.83585644461,
    3522.4688484300345,
    96533.76080639324,
    24696.29074524807,
    -38936.63081162773,
    -23840.635072629706
});

inline HeuristicMetrics beetleMetrics({
    -292118.4826934236,
    42682.09004360313,
    4481.178845826221,
    17665.014704305027,
    -30118.546365364382,
    -330519.2236168998,
    -15571.384868146333
});

inline HeuristicMetrics grasshopperMetrics({
    -62143.66709528388,
    -37382.75816433833,
    7376.7071386644275,
    81774.27711240177,
    4185.887177023414,
    -46514.16457826511,
    -29599.124484381024
});

inline HeuristicMetrics antMetrics({
    150335.73123164993,
    -54347.58400587852,
    2181.3845855769014,
    -12190.782082844375,
    -2768.31388448452,
    -24781.768062011586,
    -124949.7080146349
});

inline HeuristicMetrics pillbugMetrics({
    -232.27427974496985,
    20613.21820706331,
    141.4615857378271,
    -67297.28650806811,
    -6240.401501573603,
    3257.6537423248556,
    21863.383447357566
});

inline HeuristicMetrics mosquitoMetrics({
    147014.737145555, // InPlayWeight
    -2891.0586433755066, // IsPinnedWeight
    274.477890120718, // IsCoveredWeight
    7418.816073993939, // NoisyMoveWeight
    -2217.07900807487, // QuietMoveWeight
    5877.374514571279, // FriendlyNeighborWeight
    12195.85956817655 // EnemyNeighborWeight
});

inline HeuristicMetrics ladybugMetrics({
    -4494.037180443046, // InPlayWeight
    -5543.12988753058, // IsPinnedWeight
    -567.9664126098779, // IsCoveredWeight
    -95.67674761665349, // NoisyMoveWeight
    1006.6760051760265, // QuietMoveWeight
    1601.0593338368017, // FriendlyNeighborWeight
    -4767.602134795075 // EnemyNeighborWeight
});

inline HeuristicMetrics getMetrics(const Pieces_t piece) {
    switch (piece) {
        case W_QUEEN:
        case B_QUEEN:
            return queenMetrics;
        case W_LADYBUG:
        case B_LADYBUG:
            return ladybugMetrics;
        case W_ANT_1:
        case W_ANT_2:
        case W_ANT_3:
        case B_ANT_1:
        case B_ANT_2:
        case B_ANT_3:
            return antMetrics;
        case W_PILLBUG:
        case B_PILLBUG:
            return pillbugMetrics;
        case W_MOSQUITO:
        case B_MOSQUITO:
            return mosquitoMetrics;
        case W_BEETLE_1:
        case W_BEETLE_2:
        case B_BEETLE_1:
        case B_BEETLE_2:
            return beetleMetrics;
        case W_GRASSHOPPER_1:
        case W_GRASSHOPPER_2:
        case W_GRASSHOPPER_3:
        case B_GRASSHOPPER_1:
        case B_GRASSHOPPER_2:
        case B_GRASSHOPPER_3:
            return grasshopperMetrics;
        case W_SPIDER_1:
        case W_SPIDER_2:
        case B_SPIDER_1:
        case B_SPIDER_2:
            return spiderMetrics;
        case NULLPIECE:
            // printf("ERROR: Non-existing metrics for nullpiece\n");
    }

    // printf("ERROR: No known piece\n");

    return HeuristicMetrics({0});;
}

double mzingaHeuristic(Context_t* context);
uint_fast8_t getMovesSize(const Piece_t *moves);
void setHeuristicParams(const Context_t *context, torch::Tensor &x);

#endif //HEURISTICS_H
