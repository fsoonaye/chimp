// opening.h
#pragma once

#include "../include/chess.hpp"
#include <unordered_map>

using namespace chess;

struct BookEntry {
    Move move;
    int  weight;
};

class OpeningBook {
    std::unordered_map<uint64_t, std::vector<BookEntry>> book;
    int                                                  maxMoves;

   public:
    OpeningBook() :
        maxMoves(5) {}

    void loadFromFile(const std::string& filename);
    bool probe(const Board& board, Move& result) const;
};