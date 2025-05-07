#pragma once
#include "chess.hpp"

using namespace chess;

constexpr int MAX_PLY = 64;

constexpr int VALUE_MATE = 32000;
constexpr int VALUE_INF  = VALUE_MATE + 1;
constexpr int VALUE_NONE = VALUE_MATE + 2;

constexpr int VALUE_MATE_IN_PLY  = VALUE_MATE - MAX_PLY;
constexpr int VALUE_MATED_IN_PLY = -VALUE_MATE_IN_PLY;

constexpr int DEPTH_QS = 0;

/**
 * @brief Checks if a score indicates a checkmate
 * @param score Score to check
 * @return True if the score represents a checkmate
 */
inline bool is_mate(int score) { return std::abs(score) >= VALUE_MATE - MAX_PLY; }

/**
 * @brief Calculates a mate score at a specific ply
 * @param ply Distance from root position
 * @return Score indicating checkmate at the given ply
 */
inline int mate_in(int ply) { return VALUE_MATE - ply; }

/**
 * @brief Calculates a mated score at a specific ply
 * @param ply Distance from root position
 * @return Score indicating being checkmated at the given ply
 */
inline int mated_in(int ply) { return ply - VALUE_MATE; }

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