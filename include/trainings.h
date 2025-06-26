//
// Created by filip on 20/05/2025.
//

#ifndef TRAININGS_H
#define TRAININGS_H

#include "hivecnn.h"
#include <cstdlib>
#include <array>
#include "heuristics.h"
#include <type_traits>

class Trainer {
    std::string filename_;

public:
    explicit Trainer(std::string filename) {
        filename_ = std::string(filename);
    }

    virtual ~Trainer() = default;

    virtual void train(bool toLoad) = 0;

    const std::string &getFilename() const { return filename_; }
};

class LearnFromHeuristicTrainer : public Trainer {
public:
    explicit LearnFromHeuristicTrainer(const std::string &filename) : Trainer(filename) {
    }

    virtual double heuristic(Context_t *context) = 0; // Values must be saturated from -1 to 1
    void train(bool toLoad) override;
};

class SelfPlayTrainer : public Trainer {
public:
    explicit SelfPlayTrainer(const std::string &filename) : Trainer(filename) {
    }

    void train(bool toLoad) override;
};

class MzingaHeuristicTrainer : public LearnFromHeuristicTrainer {
public:
    explicit MzingaHeuristicTrainer(const std::string &filename) : LearnFromHeuristicTrainer(filename) {
    }

    double heuristic(Context_t *context) override;
};

inline void trainHeur() {
    MzingaHeuristicTrainer mzinga = MzingaHeuristicTrainer("model_checkpoint.pt");
    mzinga.train(false);
}

inline void trainSelfPlay(const bool toLoad) {

    auto self_play_trainer = SelfPlayTrainer("model_checkpoint");
    self_play_trainer.train(toLoad);

}

#endif //TRAININGS_H