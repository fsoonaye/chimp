#pragma once

#include "../include/chess.hpp"

using namespace chess;

constexpr int VALUE_INF = 32000;
constexpr int MAX_DEPTH = 64;

struct Time {
    int64_t optimum = 0;
    int64_t maximum = 0;
};

struct Limits {
    Time     time;
    uint64_t nodes      = 0;
    int      depth      = MAX_DEPTH - 1;
    bool     isInfinite = false;
};