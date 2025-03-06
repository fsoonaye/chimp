#pragma once

#include "../include/chess.hpp"

using namespace chess;

struct Time {
    int64_t optimum = 0;
    int64_t maximum = 0;
};

struct Limits {
    Time     time;
    uint64_t nodes      = 0;
    int      depth      = constants::MAX_MOVES - 1;
    bool     isInfinite = false;
    int      movestogo  = 0;
    int64_t  wtime      = 0;
    int64_t  btime      = 0;
    int64_t  winc       = 0;
    int64_t  binc       = 0;
};