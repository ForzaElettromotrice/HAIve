//
// Created by filip on 27/06/2025.
//

#ifndef MINMANAGER_H
#define MINMANAGER_H

#include <cstring>
#include <string>
#include <vector>
#include <fstream>
extern "C" {
#include "enums.h"
#include "utils.h"
}

class MinManager {

    std::string filename_;
    std::vector<Piece_t> moves_;
    Context_t context_ = {};
    int result_ = 2;

    void initMinManager();

public:

    MinManager() = default;

    void load(const std::string &filename) : filename_(filename) {
        initMinManager();
    }

    [[nodiscard]] bool isEnded() const {
        return moves_.empty();
    }

    Context_t* getNext();
    [[nodiscard]] int result() const {
        return result_;
    }

};

#endif //MINMANAGER_H
