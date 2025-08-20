//
// Created by filippo on 15/06/25.
//

#ifndef HEURISTICS_H
#define HEURISTICS_H

extern "C" {
#include "enums.h"
}

#include "preprocessor.hpp"
#include <array>

#define paramSize 2*NUM_PIECES+2*NUM_PIECES

class HeuristicMetrics
{
    std::array<double, 7> values_;

public:

    HeuristicMetrics() {
        values_ = std::array<double, 7>();
    }

    explicit HeuristicMetrics(const std::array<double, 7> &values) : values_(values)
    {}

    explicit HeuristicMetrics(const std::vector<double>& values) {
        values_ = std::array<double, 7>();
        for (int i = 0; i < 7; i++) {
            values_[i] = values[i];
        }
    }

    double inPlayWeight() const
    {
        return values_[0];
    }

    double isPinnedWeight() const
    {
        return values_[1];
    }

    double isCoveredWeight() const
    {
        return values_[2];
    }

    double noisyMoveWeight() const
    {
        return values_[3];
    }

    double quietMoveWeight() const
    {
        return values_[4];
    }

    double friendlyNeighborWeight() const
    {
        return values_[5];
    }

    double enemyNeighborWeight() const
    {
        return values_[6];
    }

    HeuristicMetrics enemy() const
    {
        auto new_values_ = std::array<double, 7>();
        for (uint8_t i = 0; i < 7; i++)
        {
            new_values_[i] = -values_[i];
        }
        return HeuristicMetrics(new_values_);
    }

    std::array<double, 7> weights() const {
        return values_;
    }
};

class MetricsManager {
public:

    MetricsManager(){
        loadFromFile(METRICS_PATH);
    }

    MetricsManager(const MetricsManager& toCopy){
        queenMetrics = toCopy.queenMetrics;
        spiderMetrics = toCopy.spiderMetrics;
        beetleMetrics = toCopy.beetleMetrics;
        grasshopperMetrics = toCopy.grasshopperMetrics;
        antMetrics = toCopy.antMetrics;
        pillbugMetrics = toCopy.pillbugMetrics;
        mosquitoMetrics = toCopy.mosquitoMetrics;
        ladybugMetrics = toCopy.ladybugMetrics;
    }

    MetricsManager& operator=(const MetricsManager& toCopy) {
        if (this != &toCopy) {
            queenMetrics = toCopy.queenMetrics;
            spiderMetrics = toCopy.spiderMetrics;
            beetleMetrics = toCopy.beetleMetrics;
            grasshopperMetrics = toCopy.grasshopperMetrics;
            antMetrics = toCopy.antMetrics;
            pillbugMetrics = toCopy.pillbugMetrics;
            mosquitoMetrics = toCopy.mosquitoMetrics;
            ladybugMetrics = toCopy.ladybugMetrics;
        }
        return *this;
    }

    HeuristicMetrics& getMetrics(const Pieces_t piece) {
        switch (piece)
        {
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
            default: return queenMetrics;
        }
    };
    bool saveToFile(const std::string& filename) const;


private:
    bool loadFromFile(const std::string& filename);
    HeuristicMetrics queenMetrics;
    HeuristicMetrics spiderMetrics;
    HeuristicMetrics beetleMetrics;
    HeuristicMetrics grasshopperMetrics;
    HeuristicMetrics antMetrics;
    HeuristicMetrics pillbugMetrics;
    HeuristicMetrics mosquitoMetrics;
    HeuristicMetrics ladybugMetrics;
};

double mzingaHeuristic(HAIveContext_t *context);
uint_fast8_t getMovesSize(const Piece_t *moves);
void setHeuristicParams(const HAIveContext_t *context, torch::Tensor &x);

#endif //HEURISTICS_H
