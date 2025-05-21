#pragma once
#include "chess.hpp"

using namespace chess;

// Board configuration
constexpr int BOARD_SIZE  = 64;  // Maximum number of squares
constexpr int NUM_COLORS  = 2;   // White and black
constexpr int NUM_KILLERS = 2;   // Killer moves per ply

// Search bounds
constexpr int MAX_PLY   = 64;   // Maximum search depth
constexpr int MAX_MOVES = 128;  // Maximum legal moves per position

// Evaluation bounds
constexpr int VALUE_MATE         = 32000;                 // Checkmate score
constexpr int VALUE_INF          = VALUE_MATE + 1;        // Infinity
constexpr int VALUE_NONE         = VALUE_MATE + 2;        // No value
constexpr int VALUE_MATE_IN_PLY  = VALUE_MATE - MAX_PLY;  // Mate distance bonus
constexpr int VALUE_MATED_IN_PLY = -VALUE_MATE_IN_PLY;    // Mated distance penalty

// Transposition table
constexpr int DEPTH_QS = 0;  // Depth value for quiescence search entries in TT

// History table
constexpr int MAX_HISTORY_VALUE = 16384;  // Maximum value for an entry in the history table

/**
 * @brief Checks if a score indicates a checkmate
 * @param score Score to check
 * @return True if the score represents a checkmate
 */
constexpr bool is_mate(int score) { return std::abs(score) >= VALUE_MATE_IN_PLY; }

/**
 * @brief Checks if a score indicates a loss
 * @param score Score to check
 * @return True if the score represents a loss
 */
constexpr bool is_loss(int score) { return score <= VALUE_MATED_IN_PLY; }

/**
 * @brief Checks if a score indicates a win
 * @param score Score to check
 * @return True if the score represents a win
 */
constexpr bool is_win(int score) { return score >= VALUE_MATE_IN_PLY; }

/**
 * @brief Calculates a mate score at a specific ply
 * @param ply Distance from root position
 * @return Score indicating checkmate at the given ply
 */
constexpr int mate_in(int ply) { return VALUE_MATE - ply; }

/**
 * @brief Calculates a mated score at a specific ply
 * @param ply Distance from root position
 * @return Score indicating being checkmated at the given ply
 */
constexpr int mated_in(int ply) { return ply - VALUE_MATE; }

/**
 * @enum Bound
 * @brief Type of bound for transposition table entries
 */
enum Bound : uint8_t {
    BOUND_NONE,   ///< No bound information
    BOUND_UPPER,  ///< Upper bound (fail-low result, beta cutoff)
    BOUND_LOWER,  ///< Lower bound (fail-high result, alpha unchanged)
    BOUND_EXACT   ///< Exact score
};

/**
 * @struct Time
 * @brief Time allocation information for a search
 */
struct Time {
    int64_t optimum = 0;  ///< Target search time
    int64_t maximum = 0;  ///< Maximum allowed search time
};

/**
 * @struct Limits
 * @brief Search limit parameters
 */
struct Limits {
    Time     time;
    uint64_t nodes      = 0;
    int      depth      = MAX_PLY;
    bool     isInfinite = false;  // not handled yet
};

/**
 * @struct SearchInfo
 * @brief Stores search information for each ply during negamax
 */
struct SearchInfo {
    Move currmove = Move::NO_MOVE;  ///< Current move being searched
    int  eval     = VALUE_NONE;     ///< Static evaluation at this position
};

struct Stack {
    int      eval        = VALUE_NONE;
    int      movecount   = 0;
    Move     currmove    = Move::NO_MOVE;
    bool     is_in_check = false;
    uint16_t ply         = 0;
};