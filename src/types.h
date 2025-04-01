#pragma once

#include "../include/chess.hpp"

using namespace chess;

constexpr int MAX_PLY = 64;

constexpr int VALUE_MATE        = 32000;
constexpr int VALUE_MATE_IN_MAX = VALUE_MATE - 999;
constexpr int VALUE_INF         = VALUE_MATE + 1;
constexpr int VALUE_NONE        = VALUE_MATE + 2;

inline bool is_mate(int score) { return std::abs(score) >= VALUE_MATE - MAX_PLY; }
inline int  mate_in(int ply) { return VALUE_MATE - ply; }
inline int  mated_in(int ply) { return ply - VALUE_MATE; }

enum Bound : uint8_t {
    BOUND_NONE,
    BOUND_UPPER,
    BOUND_LOWER,
    BOUND_EXACT
};

struct Time {
    int64_t optimum = 0;
    int64_t maximum = 0;
};

struct Limits {
    Time     time;
    uint64_t nodes      = 0;
    int      depth      = MAX_PLY - 1;
    bool     isInfinite = false;
};