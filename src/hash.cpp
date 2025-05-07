#include "hash.h"

uint32_t TranspositionTable::index(uint64_t key) {
#ifdef __SIZEOF_INT128__
    return (uint64_t) (((__uint128_t) key * (__uint128_t) table.size()) >> 64);
#else
    return key % entries_.size();
#endif
}


void TranspositionTable::store(uint64_t key, int depth, int score, Move move, Bound bound) {
    TTEntry* tte = &table[index(key)];

    if (tte->key != key || move != Move::NO_MOVE)
        tte->move = move;

    if (tte->key != key || bound == BOUND_EXACT || depth + 2 > tte->depth)
    {
        tte->key   = key;
        tte->depth = depth;
        tte->score = score;
        tte->bound = bound;
    }
}


TTEntry* TranspositionTable::probe(uint64_t key, Move& ttmove, bool& tt_hit) {
    TTEntry* tte = &table[index(key)];
    tt_hit       = (tte->key == key);
    ttmove       = tte->move;
    return tte;
}


void TranspositionTable::clear() { std::fill(table.begin(), table.end(), TTEntry()); }


TranspositionTable::TranspositionTable() { allocateMB(64); }


void TranspositionTable::allocateMB(uint64_t size_mb) {
    uint64_t sizeB = size_mb * static_cast<int>(1e6);
    table.resize(sizeB / sizeof(TTEntry));
    std::cout << "hash set to " << sizeB / 1e6 << " MB" << std::endl;
}