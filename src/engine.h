#pragma once
#include "../include/chess.hpp"
#include "types.h"
#include "hash.h"
#include <chrono>

using namespace chess;

class Engine {
   public:
    // search functions
    Move get_bestmove(int depth = MAX_DEPTH);
    Move iterative_deepening(int max_depth);
    int  absearch(int alpha, int beta, int depth, int ply);
    int  quiescence_search(int alpha, int beta, int depth, int ply);

    // reset function for ucinewgame
    void reset() {
        board = Board::fromFen(constants::STARTPOS);
        nodes = 0;
        tt    = TranspositionTable();
    }

    // time functions
    bool time_is_up() {
        int64_t elapsed = get_elapsedtime();
        if (limits.time.optimum != 0)
            return elapsed >= limits.time.optimum;

        return false;
    }

    int64_t get_elapsedtime() const {
        auto currtime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(currtime - starttime).count();
    }

    // print functions
    void print_search_info(int depth, int score, uint64_t nodes, int64_t time_ms) {
        std::cout << "info";
        std::cout << " depth " << depth;
        std::cout << " score cp " << score;  // Centipawn score
        std::cout << " nodes " << nodes;
        std::cout << " time " << time_ms;
        std::cout << " nps " << (time_ms > 0 ? (nodes * 1000) / time_ms : 0);  // Nodes per second
        std::cout << std::endl;
    }

    uint64_t nodes = 0;

    Board board;

    Limits limits;

    std::chrono::high_resolution_clock::time_point starttime;

    TranspositionTable tt{};
};