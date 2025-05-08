#include "engine.h"
#include "types.h"    // For MAX_PLY, Move::NO_MOVE, constants::STARTPOS, VALUE_MATE, is_mate
#include <cstring>    // For std::memset
#include <algorithm>  // For std::fill
#include <chrono>     // For std::chrono functions
#include <sstream>    // For std::stringstream
#include <iostream>   // For std::cout, std::endl
#include <string>     // For std::string
#include <cmath>      // For std::abs

// Bring chess namespace into scope for this file if not already by engine.h
// For safety, it's good practice to have it in the .cpp if relying on it.
// However, if engine.h already has `using namespace chess;` at global scope,
// this might be redundant but harmless.
// If engine.h only has it inside its own namespace or class, then it's needed here.
// Assuming it's effectively available from engine.h or types.h for now.

// Definitions for Engine class member functions will go here.

void Engine::reset() {
    board       = Board::fromFen(constants::STARTPOS);
    nodes       = 0;
    stop_search = false;
    tt.clear();
    init_heuristic_tables();
}

bool Engine::time_is_up() {
    if (stop_search)
        return true;

    // handle go nodes <x>
    if (limits.nodes > 0 && nodes >= limits.nodes)
    {
        stop_search = true;
        return true;
    }

    // Checking time every node is costly.
    // Instead, this bitmask trick is used to check only every 2048 nodes
    if ((nodes & 2047) != 2047)
        return false;

    int64_t elapsed = get_elapsedtime();

    // to handle bench properly
    if (limits.time.maximum != 0)
    {
        // Hard limit
        if (elapsed >= limits.time.maximum)
        {
            stop_search = true;
            return true;
        }
    }

    return false;
}

int64_t Engine::get_elapsedtime() const {
    auto currtime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(currtime - starttime).count();
}

void Engine::init_heuristic_tables() {
    std::memset(pv_length, 0, sizeof(pv_length));
    std::fill(&pv_table[0][0], &pv_table[0][0] + MAX_PLY * MAX_PLY, Move::NO_MOVE);
    std::fill(&killer_moves[0][0], &killer_moves[0][0] + MAX_PLY * 2, Move::NO_MOVE);
    std::memset(history_table, 0, sizeof(history_table));
}

std::string Engine::get_pv_string() {
    std::stringstream ss;

    for (int i = 0; i < pv_length[0]; i++)
        ss << pv_table[0][i] << " ";

    return ss.str();
}

void Engine::print_search_info(int depth, int score, uint64_t nodes, int64_t time_ms) {
    if (!debug)
        return;

    std::string score_type;
    int         score_value;

    if (is_mate(score))
    {
        score_type        = "mate";
        int mate_distance = (VALUE_MATE - std::abs(score) + 1) / 2;
        score_value       = score > 0 ? mate_distance : -mate_distance;
    }
    else
    {
        score_type  = "cp";
        score_value = score;
    }

    std::cout << "info";
    std::cout << " depth " << depth;
    std::cout << " score " << score_type << " " << score_value;
    std::cout << " nodes " << nodes;
    std::cout << " time " << time_ms;
    std::cout << " nps " << (time_ms > 0 ? (nodes * 1000) / time_ms : 0);
    std::cout << " pv " << get_pv_string();
    std::cout << std::endl;
}

void Engine::init_reduction_table() {
    for (int depth = 1; depth < MAX_PLY; ++depth)
        for (int movecount = 1; movecount < MAX_MOVES; ++movecount)
            reduction_table[depth][movecount] =
              static_cast<int>(1
                               + std::log(static_cast<double>(depth))
                                   * std::log(static_cast<double>(movecount)) / 2.25);
}