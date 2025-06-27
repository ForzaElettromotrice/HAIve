//
// Created by filip on 27/06/2025.
//

#include "minmanager.h"
void MinManager::initMinManager() {
    {

        moves_ = std::vector<Piece_t>();
        initContext(&context_);

        std::ifstream is(filename_, std::ios::binary);
        if (!is)
            throw std::runtime_error("Unable to open file\n");

        std::string line;

        // first line
        std::getline(is, line);
        result_ = std::stoi(line);
        if (result_ > 1 || result_ < -1)
            throw std::runtime_error("Invalid play result\n");

        // other line
        while (std::getline(is, line)) {
            char* mutable_line = new char[line.size() + 1];  // +1 for null terminator
            std::strcpy(mutable_line, line.c_str());

            Piece_t p = parseMove(context_.idToPos, mutable_line);
            moves_.push_back(p);

            addOurMove(&context_, &p);
        }

        resetContext(&context_);

    }
}

Context_t *MinManager::getNext() {

    addOurMove(&context_, &moves_[0]);
    moves_.erase(moves_.begin());
    return &context_;

}

