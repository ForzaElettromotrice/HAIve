//
// Created by filip on 14/05/2025.
//

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

extern "C" {
#include "enums.h"
#include "utils.h"
#include "moves.h"
#include "hashmap.h"
}

#include "string"
#include <limits>
#include <torch/torch.h>
#include "fstream"

typedef struct ProcessorArgs
{
    torch::Tensor x;
    const Position_t *positions;
} ProcessorArgs_t;

enum class Result : int8_t
{
    RESULT_BLACK_WON = -2,
    RESULT_DRAW = 0,
    RESULT_WHITE_WON = 2,
};

enum class Layer : uint8_t
{
    QUEEN = 0,
    ANT = 1,
    BEETLE_1 = 2,
    BEETLE_2 = 3,
    BEETLE_3 = 4,
    BEETLE_4 = 5,
    BEETLE_5 = 6,
    BEETLE_6 = 7,
    SPIDER = 8,
    GRASSHOPPER = 9,
    MOSQUITO_1 = 10,
    MOSQUITO_2 = 11,
    MOSQUITO_3 = 12,
    MOSQUITO_4 = 13,
    MOSQUITO_5 = 14,
    MOSQUITO_6 = 15,
    LADYBUG = 16,
    PILLBUG = 17
};

constexpr uint8_t sizeLayer = 18;

class Processor
{
    std::string &fileName_;
    std::vector<std::vector<Position_t> > positionSequence_ = std::vector<std::vector<Position_t> >();
    GameStatus_t gameStatus_;
    torch::Tensor currentTensor_ = torch::Tensor();

public:
    static void saveToFile(std::ofstream &os, const Position_t *positions, const Result &result);
    static void loadFromFile(std::ifstream &is, Result &result);
    static void multipleSaveToFile(const std::string &fileName, const std::vector<std::vector<Position_t> > &allPositions, const std::vector<Result> &results);
    static void multipleSaveToFile(const std::string &fileName, const std::vector<std::vector<Position_t> > &allPositions, const Result &result);
    static void boardToTensor(const Position_t *positions, torch::Tensor &tensor);
    static void *boardToTensor_mt(void *args);

    explicit Processor(std::string &fileName);
    void newGame();
    void playMove(const HAIveContext_t *context);
    void endGame(const HAIveContext_t *context, const Result &result);
    torch::Tensor &getTensor();
};

#endif //PREPROCESSOR_H
