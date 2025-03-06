// hash.h
#pragma once

#include "../include/chess.hpp"
#include <cstdint>
#include <vector>

using namespace chess;

class TT {
   public:
    enum class Bound {
        EXACT,
        LOWER,
        UPPER
    };

    struct TTEntry {
        uint64_t hash     = 0;
        int      score    = 0;
        Move     bestMove = Move::NO_MOVE;
        uint8_t  depth    = 0;
        Bound    bound    = Bound::EXACT;

        TTEntry() = default;
        TTEntry(uint64_t h, int s, Bound b, uint8_t d, Move m) :
            hash(h),
            score(s),
            bestMove(m),
            depth(d),
            bound(b) {}
    };

    TT(size_t size) :
        entries(size),
        mask(size - 1) {
        assert((size & (size - 1)) == 0);  // Ensure size is power of two
    }

    void store(uint64_t hash, int score, Bound bound, uint8_t depth, Move bestMove) {
        size_t idx = hash & mask;
        // Replace if deeper or same depth but more recent
        if (entries[idx].depth <= depth)
            entries[idx] = TTEntry(hash, score, bound, depth, bestMove);
    }

    TTEntry probe(uint64_t hash) const {
        size_t idx = hash & mask;
        return entries[idx];
    }

    void clear() { std::fill(entries.begin(), entries.end(), TTEntry()); }

   private:
    std::vector<TTEntry> entries;
    size_t               mask;
};