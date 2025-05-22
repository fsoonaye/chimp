#include "engine.h"
#include "types.h"
#include <cstring>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iostream>
#include <string>
#include <cmath>


void Engine::update_quiet_heuristics(Move move, int ply, int depth) {
    // KILLER MOVE UPDATE
    if (move != killer_moves[ply][0])
    {
        killer_moves[ply][1] = killer_moves[ply][0];
        killer_moves[ply][0] = move;
    }

    // HISTORY HEURISTICS UPDATE
    int& entry = history_table[board.sideToMove()][move.from().index()][move.to().index()];
    int  bonus = depth * depth;

    int clamped_bonus = std::clamp(bonus, -MAX_HISTORY_VALUE, MAX_HISTORY_VALUE);
    entry += clamped_bonus - entry * clamped_bonus / MAX_HISTORY_VALUE;
}

int Engine::get_reduction(
  int depth, int movecount, bool improving, bool is_pv_node, bool is_capture) {
    return reduction_table[depth][movecount] + improving - is_pv_node - is_capture;
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

void Engine::reset() {
    board       = Board::fromFen(constants::STARTPOS);
    nodes       = 0;
    stop_search = false;
    tt.clear();
    init_tables();
}

void Engine::init_tables() {
    // Initialize principal variation tables
    std::memset(pv_length, 0, sizeof(pv_length));
    std::fill(&pv_table[0][0], &pv_table[0][0] + MAX_PLY * MAX_PLY, Move::NO_MOVE);

    // Initialize killer moves table
    std::fill(&killer_moves[0][0], &killer_moves[0][0] + MAX_PLY * 2, Move::NO_MOVE);

    // Initialize history heuristics table
    std::memset(history_table, 0, sizeof(history_table));

    // Initialize counter moves table
    std::fill(&counter_moves[0][0], &counter_moves[0][0] + 64 * 64, Move::NO_MOVE);

    // Initialize late move reduction table
    for (int depth = 1; depth < MAX_PLY; ++depth)
        for (int movecount = 1; movecount < MAX_MOVES; ++movecount)
            reduction_table[depth][movecount] =
              1 + static_cast<int>(std::log(depth) * std::log(movecount) / 2.25);
}