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
    void reset();

    /**
     * @brief Checks if the search should be terminated based on limits
     * @return True if search should stop, false otherwise
     */
    bool time_is_up();

    /**
     * @brief Returns elapsed time since search start in milliseconds
     * @return Elapsed time in milliseconds
     */
    int64_t get_elapsedtime() const;

    /**
     * @brief Initializes tables used for move ordering and search heuristics
     * 
     * Clears the principal variation tables and killer moves table.
     */
    void init_heuristic_tables();

    /**
     * @brief Generates a string representation of the principal variation
     * @return String containing the sequence of best moves
     */
    std::string get_pv_string();

    /**
     * @brief Outputs information about the current search to the console
     * @param depth Current search depth
     * @param score Best score found at this depth
     * @param nodes Total number of nodes searched since depth 1
     * @param time_ms Time spent searching in milliseconds since depth 1
     */
    void print_search_info(int depth, int score, uint64_t nodes, int64_t time_ms);

    /**
     * @brief Initializes the late move reduction table.
     */
    void init_reduction_table();


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
