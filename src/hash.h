#pragma once
#include "../include/chess.hpp"
#include "types.h"

using namespace chess;

struct TTEntry {
    uint64_t key   = 0;
    int      depth = 0;
    int      score = 0;
    Move     move  = Move::NO_MOVE;
    Bound    bound = BOUND_NONE;
};

class TranspositionTable {
   public:
    TranspositionTable();
    std::vector<TTEntry> table;

    void     store(uint64_t key, int depth, int score, Move move, Bound bound);
    TTEntry* probe(uint64_t key, Move& ttmove, bool& tt_hit);

    uint32_t index(uint64_t key);


    void allocate(uint64_t size);
    void allocateMB(uint64_t size_mb);
    void clear();

    static constexpr uint64_t MAXHASH_MiB = (1ull << 32) * sizeof(TTEntry) / (1024 * 1024);
};