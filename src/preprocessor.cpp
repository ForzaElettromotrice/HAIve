//
// Created by filip on 14/05/2025.
//

#include "preprocessor.h"
#include <torch/torch.h>

#include "utils.h"

enum class Layer : int {
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

uint8_t pieceToLayer(const Pieces_t pieceId, uint8_t z) {
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

torch::Tensor& boardToTensor(const Context_t* context) {

    const auto options = torch::TensorOptions().dtype(torch::kInt8);
    torch::Tensor tensor = torch::zeros({sizeLayer, BOARD_Y, BOARD_X}, options);

    const Position_t *positions = context->idToPos;

    for (uint_fast8_t i = 0; i < NUM_PIECES; i++) {
        int_fast8_t z = positions[i].z;
        if (z == -1)
            continue;
        int_fast8_t y = positions[i].y;
        int_fast8_t x = positions[i].x;
        uint8_t layerValue = pieceToLayer(static_cast<const Pieces_t>(i), positions[i].z);
        tensor.index_put_({z, yOf(y), xOf(x)}, i <= 13 ? -1 : 1);
    }

    return tensor;
}
