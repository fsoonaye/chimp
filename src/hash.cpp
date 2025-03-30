#include "hash.h"

void TranspositionTable::store(uint64_t key, int depth, int score, Move move) {
    TTEntry* tte = &table[index(key)];

    if (tte->key != key || depth >= tte->depth)
    {
        tte->key   = key;
        tte->depth = depth;
        tte->score = score;

        if (move != Move::NO_MOVE)
            tte->move = move;
    }
}

void TranspositionTable::clear() {
    for (auto& entry : table)
        entry = TTEntry();
}

TTEntry* TranspositionTable::probe(uint64_t key, Move& ttmove, bool& tt_hit) {
    TTEntry* tte = &table[index(key)];
    tt_hit       = (tte->key == key);
    ttmove       = tte->move;
    return tte;
}

TranspositionTable::TranspositionTable() { allocateMB(64); }

void TranspositionTable::allocate(uint64_t size) { table.resize(size, TTEntry()); }

void TranspositionTable::allocateMB(uint64_t size_mb) {
    uint64_t sizeB    = size_mb * static_cast<int>(1e6);
    sizeB             = std::clamp(sizeB, uint64_t(1), uint64_t(MAXHASH_MiB * 1e6));
    uint64_t elements = sizeB / sizeof(TTEntry);
    allocate(elements);
    std::cout << "hash set to " << sizeB / 1e6 << " MB" << std::endl;
}

uint32_t TranspositionTable::index(uint64_t key) { return key % table.size(); }