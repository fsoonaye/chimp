#pragma once

#include "../include/chess.hpp"

using namespace chess;

constexpr int MAX_DEPTH = 64;

constexpr int VALUE_MATE        = 32000;
constexpr int VALUE_MATE_IN_MAX = VALUE_MATE - 999;
constexpr int VALUE_INF         = VALUE_MATE + 1;
constexpr int VALUE_NONE        = VALUE_MATE + 2;

inline bool is_mate(int score) { return std::abs(score) >= VALUE_MATE - MAX_DEPTH; }

struct Time {
    int64_t optimum = 0;
    int64_t maximum = 0;
};

struct Limits {
    Time     time;
    uint64_t nodes      = 0;
    int      depth      = MAX_DEPTH - 1;
    bool     isInfinite = false;
    int      movestogo  = 0;
    int64_t  wtime      = 0;
    int64_t  btime      = 0;
    int64_t  winc       = 0;
    int64_t  binc       = 0;
};