#pragma once
#include "../include/chess.hpp"
#include "types.h"

using namespace chess;

class Engine {
   public:
    Move get_randommove();
    Move get_bestmove(int depth = MAX_DEPTH);
    Move iterative_deepening(int max_depth);
    int  absearch(int alpha, int beta, int depth);

    bool time_is_up();

    void reset() {
        board = Board::fromFen(constants::STARTPOS);
        nodes = 0;
    }

    int64_t get_elapsedtime() const {
        auto currtime = std::__1::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(currtime - starttime).count();
    }


    int nodes = 0;

    Board board;

    Limits limits;

    std::__1::chrono::high_resolution_clock::time_point starttime;
};