//
// Created by filip on 14/05/2025.
//

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

extern "C"{
#include "enums.h"
#include "utils.h"
    }
#include "string"
#include "fstream"
#include <torch/torch.h>

enum class Result : int8_t {
    RESULT_BLACK_WON = -1,
    RESULT_DRAW = 0,
    RESULT_WHITE_WON = 1
};

enum class Layer : uint8_t {
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

inline uint8_t pieceToLayer(const Pieces_t pieceId, const uint8_t z) {
    switch (pieceId) {
        case W_QUEEN:
        case B_QUEEN:
            return static_cast<uint8_t>(Layer::QUEEN);
        case W_BEETLE_1:
        case B_BEETLE_1:
        case W_BEETLE_2:
        case B_BEETLE_2:
            return static_cast<uint8_t>(Layer::BEETLE_1) + z;
        case W_SPIDER_1:
        case B_SPIDER_1:
        case W_SPIDER_2:
        case B_SPIDER_2:
            return static_cast<uint8_t>(Layer::SPIDER);
        case W_PILLBUG:
        case B_PILLBUG:
            return static_cast<uint8_t>(Layer::PILLBUG);
        case W_LADYBUG:
        case B_LADYBUG:
            return static_cast<uint8_t>(Layer::LADYBUG);
        case W_MOSQUITO:
        case B_MOSQUITO:
            return static_cast<uint8_t>(Layer::MOSQUITO_1) + z;
        case W_ANT_1:
        case B_ANT_1:
        case W_ANT_2:
        case B_ANT_2:
        case W_ANT_3:
        case B_ANT_3:
            return static_cast<uint8_t>(Layer::ANT);
        case W_GRASSHOPPER_1:
        case B_GRASSHOPPER_1:
        case W_GRASSHOPPER_2:
        case B_GRASSHOPPER_2:
        case W_GRASSHOPPER_3:
        case B_GRASSHOPPER_3:
            return static_cast<uint8_t>(Layer::GRASSHOPPER);
        case NULLPIECE:
            throw std::runtime_error("NULLPIECE cannot be put in Layer\n");
    }
    throw std::runtime_error("Unknown piece\n");
}

class Processor {
    const std::string& fileName_;
    std::vector<std::vector<Position_t>> positionSequence_ = std::vector<std::vector<Position_t>>();
    GameStatus_t gameStatus_;
    torch::Tensor currentTensor_ = torch::Tensor();

    static void saveToFile(std::ofstream& os, const Position_t* positions, const Result& result);
    static void loadFromFile(std::ifstream& is, std::vector<Position_t>& positions, Result& result);
    static void multipleLoadFromFile(const std::string& fileName, std::vector<std::vector<Position_t>>& positions, std::vector<Result>& results);
    static void multipleSaveToFile(const std::string& fileName, const std::vector<std::vector<Position_t>>& allPositions, const std::vector<Result>& results);
    static void multipleSaveToFile(const std::string& fileName, const std::vector<std::vector<Position_t>>& allPositions, const Result& result);
    static void boardToTensor(const Position_t *positions, torch::Tensor& tensor);

public:

    Processor(const std::string& fileName);
    void newGame();
    void playMove(const Context_t* context);
    void endGame(const Context_t* context, const Result& result);
    torch::Tensor& getTensor();
    static void getTensor(const Context_t* context, torch::Tensor& tensor);
};

#endif //PREPROCESSOR_H
