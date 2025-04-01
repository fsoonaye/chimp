#pragma once
#include "../include/chess.hpp"
#include "types.h"
#include "hash.h"
#include <chrono>

using namespace chess;

enum NodeType {
    PV,
    NON_PV,
    ROOT
};

class Engine {
   public:
    // search functions
    Move get_bestmove(int depth = MAX_PLY);
    Move iterative_deepening(int MAX_PLY);

    template<NodeType nodetype>
    int negamax_search(int alpha, int beta, int depth, int ply);
    int quiescence_search(int alpha, int beta, int depth, int ply);

    // reset function for ucinewgame
    void reset() {
        board       = Board::fromFen(constants::STARTPOS);
        nodes       = 0;
        stop_search = false;
        tt.clear();
    }

    // time functions
    bool time_is_up() {
        if (stop_search)
            return true;

        // handle go nodes <x>
        if (limits.nodes > 0 && nodes >= limits.nodes)
        {
            stop_search = true;
            return true;
        }

        // Checking time every node is costly.
        // Instead, I use this bitmask trick to check every 2048 nodes
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

    int64_t get_elapsedtime() const {
        auto currtime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(currtime - starttime).count();
    }

    void init_pv_table() {
        for (int i = 0; i < MAX_PLY; i++)
        {
            pv_length[i] = 0;
            for (int j = 0; j < MAX_PLY; j++)
            {
                pv_table[i][j] = Move::NO_MOVE;
            }
        }
    }

    // print functions
    std::string get_pv_string() {
        std::stringstream ss;

        for (int i = 0; i < pv_length[0]; i++)
            ss << pv_table[0][i] << " ";

        return ss.str();
    }

    void print_search_info(int depth, int score, uint64_t nodes, int64_t time_ms) {
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

    Move pv_table[MAX_PLY][MAX_PLY];
    int  pv_length[MAX_PLY];

    uint64_t nodes = 0;

    Board board;

    Limits limits;

    bool stop_search = false;

    std::chrono::high_resolution_clock::time_point starttime;

    TranspositionTable tt{};
};