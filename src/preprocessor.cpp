//
// Created by filip on 14/05/2025.
//

#include "preprocessor.h"

    void Processor::saveToFile(std::ofstream& os, const Position_t* positions, const Result& result) {
        for (uint_fast8_t i = 0; i < NUM_PIECES; i++) {
            const int_fast8_t z = positions[i].z;
            if (z == -1)
                continue;
            const int_fast8_t y = positions[i].y;
            const int_fast8_t x = positions[i].x;
            os << i << ":(" << z << ',' << y << ',' << x << ");";
        }
        os << std::endl;
        os << static_cast<uint8_t>(result) << std::endl;
    }

    void Processor::loadFromFile(std::ifstream& is, std::vector<Position_t>& positions, Result& result) {
        positions = std::vector<Position_t>(NUM_PIECES);
        for (uint_fast8_t j = 0; j < NUM_PIECES; j++)
            positions[j].z = -1;
        int_fast8_t x, y, z, pieceId;
        while (is.peek() != '\n'){
            is >> pieceId;
            if (is.get() != ':' && is.get() != '(')
                throw std::runtime_error("Could not parse piece id\n");
            is >> z;
            if (is.get() != ',') throw std::runtime_error("Could not parse z\n");
            is >> y;
            if (is.get() != ',') throw std::runtime_error("Could not parse y\n");
            is >> x;
            if (is.get() != ')' && is.get() != ';') throw std::runtime_error("Could not parse x\n");
            Position_t p;
            p.z = z;
            p.y = y;
            p.x = x;
            positions[pieceId] = p;
        }
        is >> pieceId; result = static_cast<Result>(pieceId);
        if (is.get() != '\n') throw std::runtime_error("Could not parse pieceId\n");
    }

    void Processor::multipleLoadFromFile(const std::string& fileName, std::vector<std::vector<Position_t>>& positions, std::vector<Result>& results) {
        std::ifstream is(fileName);
        if (!is)
            throw std::runtime_error("Could not open file\n");
        positions = std::vector<std::vector<Position_t>>();
        while (is) {
            auto pos = std::vector<Position_t>();
            Result result;
            loadFromFile(is, pos, result);
            positions.push_back(pos);
            results.push_back(result);
        }
    }

    void Processor::multipleSaveToFile(const std::string& fileName, const std::vector<std::vector<Position_t>>& allPositions, const std::vector<Result>& results) {

        std::ofstream os(fileName);
        if (!os)
            throw std::runtime_error("Could not open file " + fileName);
        for (uint_fast32_t boardStatus = 0; boardStatus < allPositions.size(); boardStatus++)
        {
            saveToFile(os, allPositions[boardStatus].data(), results[boardStatus]);
        }
    }

    void Processor::multipleSaveToFile(const std::string& fileName, const std::vector<std::vector<Position_t>>& allPositions, const Result& result) {

        std::ofstream os(fileName);
        if (!os)
            throw std::runtime_error("Could not open file " + fileName);
        for (const auto &allPosition: allPositions)
        {
            saveToFile(os, allPosition.data(), result);
        }
    }

    /*
    *  Tensor può essere passato anche inizializzato, verrà fillato.
    */
    void Processor::boardToTensor(const Position_t *positions, torch::Tensor& tensor) {

        const auto options = torch::TensorOptions().dtype(torch::kInt8);
        tensor = torch::zeros({sizeLayer, BOARD_Y / 2, BOARD_X}, options);

        for (uint_fast8_t i = 0; i < NUM_PIECES; i++)
        {
            const int_fast8_t z = positions[i].z;
            if (z == -1)
                continue;
            const int_fast8_t y = yOf(positions[i].y);
            const int_fast8_t x = xOf(positions[i].x);
            uint8_t layerValue = pieceToLayer(static_cast<const Pieces_t>(i), z);
            printf("Using x %d, y %d, layerValue %d", x, y, layerValue);
            tensor.index_put_({layerValue, (y - y % 2) / 2, x}, i <= 13 ? -1 : 1);
        }
    }

    Processor::Processor(const std::string& fileName) : fileName_(fileName) {
        gameStatus_ = NOT_STARTED;
    }

    void Processor::newGame() {
        gameStatus_ = NOT_STARTED;
        positionSequence_.clear();
    }

    /*
     *  Pass the context with the new move.
     */
    void Processor::playMove(const Context_t* context) {
        gameStatus_ = IN_PROGRESS;
        Position_t *positions = context->idToPos;
        positionSequence_.emplace_back(positions, positions + NUM_PIECES);
    }

    /*
     *  End the game. The context must have the last move done.
     */
    void Processor::endGame(const Context_t* context, const Result& result){
        if (gameStatus_ != IN_PROGRESS) {
            throw std::runtime_error("Game should be in progress\n");
        }
        gameStatus_ = NOT_STARTED;
        Position_t *positions = context->idToPos;
        positionSequence_.emplace_back(positions, positions + NUM_PIECES);
        multipleSaveToFile(fileName_, positionSequence_, result);
    }

    torch::Tensor& Processor::getTensor() {
        boardToTensor(positionSequence_.back().data(), currentTensor_);
        return currentTensor_;
    }

    void Processor::getTensor(const Context_t* context, torch::Tensor& tensor) {
        boardToTensor(context->idToPos, tensor);
    }



// TODO: Optimize tensor retrieval - create it step by step
// TODO: Get tensors step by step from file
