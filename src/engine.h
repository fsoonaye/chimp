#pragma once
#include "chess.hpp"
#include "types.h"
#include "hash.h"
#include <chrono>
#include <cstring>
#include <cmath>

using namespace chess;

/**
 * @enum NodeType
 * @brief Defines the type of node in the search tree
 * 
 * Different node types receive different treatment during search,
 * affecting pruning decisions and move ordering strategies.
 */
enum Node {
    PV,
    NON_PV,
    ROOT
};

/**
 * @class Engine
 * @brief Core chess engine implementation handling search and time management
 */
class Engine {
   public:
    /**
     * @brief Calculates the best move for the current position
     * @param depth Maximum search depth (defaults to MAX_PLY)
     * @return Best move found within the given constraints
     */
    Move get_bestmove(int depth = MAX_PLY);

    /**
     * @brief Performs iterative deepening search to find the best move
     * @param MAX_PLY Maximum search depth to consider
     * @return Best move found from the latest fully searched depth
     */
    Move iterative_deepening(int MAX_PLY);

    /**
     * @brief Principal alpha-beta negamax search implementation
     *
     * called recursively until depth reaches 0
     *
     * @tparam nodetype Type of node (PV, NON_PV, ROOT) affecting search behavior
     * @param alpha Lower bound of the search window
     * @param beta Upper bound of the search window
     * @param depth Remaining search depth
     * @param ply Current distance from root position
     * @return Position score from the perspective of the side to move
     */
    template<Node node>
    int negamax_search(int alpha, int beta, int depth, int ply);

    /**
     * @brief Quiescence search to resolve tactical sequences
     *
     * Called recursively, once negamax_search() reaches a leaf node (depth 0)
     *
     * @param alpha Lower bound of the search window
     * @param beta Upper bound of the search window
     * @param depth Remaining search depth (usually negative in quiescence)
     * @param ply Current distance from root position
     * @return Stable position score after capturing sequences
     */
    template<Node node>
    int quiescence_search(int alpha, int beta, int ply);


    /**
     * @brief Resets the engine to initial state for a new game
     * 
     * called when receiving ucinewgame UCI instruction
     */
    void reset() {
        board       = Board::fromFen(constants::STARTPOS);
        nodes       = 0;
        stop_search = false;
        tt.clear();
        std::fill(&killer_moves[0][0], &killer_moves[0][0] + MAX_PLY * 2, Move::NO_MOVE);
    }

    /**
     * @brief Checks if the search should be terminated based on limits
     * @return True if search should stop, false otherwise
     */
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

    /**
     * @brief Returns elapsed time since search start in milliseconds
     * @return Elapsed time in milliseconds
     */
    inline int64_t get_elapsedtime() const {
        auto currtime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(currtime - starttime).count();
    }

    /**
     * @brief Initializes tables used for move ordering and search heuristics
     * 
     * Clears the principal variation tables and killer moves table.
     */
    void init_heuristic_tables() {
        std::memset(pv_length, 0, sizeof(pv_length));
        std::fill(&pv_table[0][0], &pv_table[0][0] + MAX_PLY * MAX_PLY, Move::NO_MOVE);
        std::fill(&killer_moves[0][0], &killer_moves[0][0] + MAX_PLY * 2, Move::NO_MOVE);
        std::memset(history_table, 0, sizeof(history_table));
    }

    /**
     * @brief Generates a string representation of the principal variation
     * @return String containing the sequence of best moves
     */
    std::string get_pv_string() {
        std::stringstream ss;

        for (int i = 0; i < pv_length[0]; i++)
            ss << pv_table[0][i] << " ";

        return ss.str();
    }

    /**
     * @brief Outputs information about the current search to the console
     * @param depth Current search depth
     * @param score Best score found at this depth
     * @param nodes Total number of nodes searched since depth 1
     * @param time_ms Time spent searching in milliseconds since depth 1
     */
    inline void print_search_info(int depth, int score, uint64_t nodes, int64_t time_ms) {
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


    void init_reduction_table() {
        for (int depth = 1; depth < MAX_PLY; ++depth)
            for (int movecount = 1; movecount < MAX_MOVES; ++movecount)
                reduction_table[depth][movecount] =
                  1 + std::log(depth) * std::log(movecount) / 2.25;
    }


    int  history_table[2][64][64];
    int  reduction_table[MAX_PLY][MAX_MOVES];
    Move pv_table[MAX_PLY][MAX_PLY];
    int  pv_length[MAX_PLY];
    Move killer_moves[MAX_PLY][2];

    uint64_t nodes = 0;

    Board board;

    Limits limits;

    bool stop_search = false;

    std::chrono::high_resolution_clock::time_point starttime;

    TranspositionTable tt{};

    bool debug = true;
};
