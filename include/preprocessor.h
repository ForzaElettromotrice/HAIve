//
// Created by filip on 14/05/2025.
//

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

extern "C" {
    #include "enums.h"
    #include "utils.h"
    #include "moves.h"
}
#include "string"
#include <limits>
#include <torch/torch.h>
#include "fstream"


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
    SPIDER = 7,
    GRASSHOPPER = 8,
    MOSQUITO_1 = 9,
    MOSQUITO_2 = 10,
    MOSQUITO_3 = 11,
    MOSQUITO_4 = 12,
    MOSQUITO_5 = 13,
    LADYBUG = 14,
    PILLBUG = 15
};

constexpr uint8_t sizeLayer = 16;

class Processor {

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

    explicit Processor(std::string &fileName);
    void newGame();
    void playMove(const Context_t *context);
    void endGame(const Context_t *context, const Result &result);
    torch::Tensor &getTensor();

};

#endif //PREPROCESSOR_H
