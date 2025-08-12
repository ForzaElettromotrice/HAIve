//
// Created by filip on 27/06/2025.
//

#include "minmanager.hpp"

/*
std::string rstrip(const std::string &s)
{
    std::string result = s;
    result.erase(std::find_if(result.rbegin(), result.rend(),
                              [](unsigned char ch)
                              {
                                  return !std::isspace(ch);
                              }).base(),
                 result.end());
    return result;
}


void MinManager::initMinManager()
{
    {
        moves_ = std::vector<Piece_t>();
        initHAIveContext(&context_);

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
        while (std::getline(is, line))
        {
            line = rstrip(line);
            char *mutable_line = new char[line.size() + 1]; // +1 for null terminator
            std::strcpy(mutable_line, line.c_str());

            Piece_t p = parseMove(context_.idToPos, mutable_line);
            if (p.position.z <= -1 && p.id != NULLPIECE)
                break;
            moves_.push_back(p);

            addHAIveMove(&context_, &p);
        }

        resetHAIveContext(&context_);
    }
}

HAIveContext_t *MinManager::getNext()
{
    addHAIveMove(&context_, &moves_[0]);
    moves_.erase(moves_.begin());
    return &context_;
}

*/