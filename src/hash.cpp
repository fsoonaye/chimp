#include "hash.h"


uint32_t TranspositionTable::index(uint64_t key) { return key % table.size(); }

void TranspositionTable::store(uint64_t key, int depth, int score, Move move, Bound bound) {
    TTEntry* tte = &table[index(key)];

    if (tte->key != key || move != Move::NO_MOVE)
        tte->move = move;

    if (tte->key != key || depth > tte->depth || bound == BOUND_EXACT)
    {
        tte->key   = key;
        tte->depth = depth;
        tte->score = score;
        tte->bound = bound;
    }
}


const TTEntry* TranspositionTable::probe(uint64_t key, Move& ttmove, bool& tt_hit) {
    const TTEntry* tte = &table[index(key)];

    tt_hit = (tte->key == key);
    ttmove = tte->move;
    return tte;
}

void TranspositionTable::clear() { std::fill(table.begin(), table.end(), TTEntry()); }

TranspositionTable::TranspositionTable() { allocateMB(64); }

void TranspositionTable::allocate(uint64_t size) { table.resize(size, TTEntry()); }

void TranspositionTable::allocateMB(uint64_t size_mb) {
    uint64_t size_B = size_mb * static_cast<int>(1e6);
    allocate(size_B / sizeof(TTEntry));
    std::cout << "hash set to " << size_B / 1e6 << " MB" << std::endl;
}
