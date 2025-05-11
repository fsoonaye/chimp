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
enum NodeType {
    PV,   // Principal Variation node - full window search
    CUT,  // Cut node - null window search
    ROOT  // Root node - full window search
};

/**
 * @class Engine
 * @brief Core chess engine implementation handling search and time management
 */
class Engine {
   public:
    // Search functions
    /**
     * @brief Calculates the best move for the current position
     * @param depth Maximum search depth (defaults to MAX_PLY)
     * @return Best move found within the given constraints
     */
    Move get_bestmove(int depth = MAX_PLY);

    /**
     * @brief Performs iterative deepening search to find the best move
     * @param max_depth Maximum search depth to consider
     * @return Best move found from the latest fully searched depth
     */
    Move iterative_deepening(int max_depth);

    /**
     * @brief Performs aspiration window search to find the best move
     * @param alpha Lower bound of the search window
     * @param beta Upper bound of the search window
     * @param depth Remaining search depth
     * @param ply Current distance from root position
     * @return Best move found within the given constraints
     */
    int aspiration_window_search(int depth, int prevscore);

    /**
     * @brief Principal alpha-beta negamax search implementation
     *
     * called recursively until depth reaches 0
     *
     * @tparam node Type of node (PV, CUT, ROOT) affecting search behavior
     * @param alpha Lower bound of the search window
     * @param beta Upper bound of the search window
     * @param depth Remaining search depth
     * @param ply Current distance from root position
     * @return Position score from the perspective of the side to move
     */
    template<NodeType node>
    int negamax_search(int alpha, int beta, int depth, int ply);

    /**
     * @brief Quiescence search to resolve tactical sequences
     *
     * Called recursively, once negamax_search() reaches a leaf node (depth 0)
     *
     * @tparam node Type of node (PV, CUT, ROOT) affecting search behavior
     * @param alpha Lower bound of the search window
     * @param beta Upper bound of the search window
     * @param depth Remaining search depth (usually negative in quiescence)
     * @param ply Current distance from root position
     * @return Stable position score after capturing sequences
     */
    template<NodeType node>
    int quiescence_search(int alpha, int beta, int ply);

    /**
     * @brief Updates killer moves and history heuristics for a quiet move causing a beta cutoff.
     * @param move The quiet move that caused the beta cutoff.
     * @param ply The current ply in the search.
     * @param depth The search depth used for the history bonus calculation.
     */
    void update_quiet_heuristics(Move move, int ply, int depth);

    // Time management functions
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

    // Print functions
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

    // Initialization functions
    /**
     * @brief Resets the engine to initial state for a new game
     * 
     * called when receiving ucinewgame UCI instruction
     */
    void reset();

    /**
     * @brief Initializes all search-related tables and data structures
     * 
     * This includes:
     * - Principal variation tables
     * - Killer moves table
     * - History heuristics table
     * - Late move reduction table
     * - Search information table
     */
    void init_tables();

    // Search tables
    int        history_table[NUM_COLORS][BOARD_SIZE][BOARD_SIZE];
    int        reduction_table[MAX_PLY][MAX_MOVES];
    int        pv_length[MAX_PLY];
    Move       pv_table[MAX_PLY][MAX_PLY];
    Move       killer_moves[MAX_PLY][NUM_KILLERS];
    Move       counter_moves[BOARD_SIZE][BOARD_SIZE];
    SearchInfo search_info[MAX_PLY + 4];

    // Search statistics and state
    // Total nodes searched
    uint64_t nodes = 0;
    // Current position
    Board board;
    // Search limits
    Limits limits;
    // Search termination flag
    bool stop_search = false;
    // Search start time
    std::chrono::high_resolution_clock::time_point starttime;
    // Transposition table
    TranspositionTable tt{};
    // Debug output flag
    bool debug = true;
};
