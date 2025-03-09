#pragma once
#include "../include/chess.hpp"
#include "types.h"

using namespace chess;

class Engine {
   public:
    Move get_bestmove(int depth);

    void reset() {
        board = Board::fromFen(constants::STARTPOS);
        nodes = 0;
    }

    int MAX_PLY = 64;
    int nodes   = 0;

    Board board;

    Limits limit;
};

std::uint64_t perft(Board& board, int depth);
void          start_perft(const std::string& fen, int depth);