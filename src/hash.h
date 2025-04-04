#pragma once
#include "../include/chess.hpp"
#include "types.h"

using namespace chess;

/**
 * @struct TTEntry
 * @brief Transposition table entry storing search results for a position
 * 
 * Each entry contains information about a previously searched position,
 * including the Zobrist hash key, search depth, score, best move, and
 * the type of bound (exact, upper, or lower).
 */
struct TTEntry {
    uint64_t key   = 0;
    int      depth = 0;
    int      score = 0;
    Move     move  = Move::NO_MOVE;
    Bound    bound = BOUND_NONE;
};

/**
 * @class TranspositionTable
 * @brief Stores and retrieves search results for previously evaluated positions
 * 
 * The transposition table uses Zobrist hashing to identify positions and 
 * implements a replacement strategy to manage table entries efficiently.
 * 
 * Heavily inspired from smallbrain for now.
 */
class TranspositionTable {
   public:
    /**
     * @brief Constructs a transposition table with default size
     */
    TranspositionTable();

    /**
     * @brief Stores a search result in the transposition table
     * @param key Zobrist hash key of the position
     * @param depth Search depth at which the result was found
     * @param score Score of the position
     * @param move Best move found for the position
     * @param bound Type of bound (exact, upper, lower)
     */
    void store(uint64_t key, int depth, int score, Move move, Bound bound);


    /**
     * @brief Probes the transposition table for a position
     * @param key Zobrist hash key of the position to look up
     * @param ttmove Reference to store the best move if found
     * @param tt_hit Reference to indicate whether a matching entry was found
     * @return Pointer to the retrieved entry or nullptr if not found
     */
    TTEntry* probe(uint64_t key, Move& ttmove, bool& tt_hit);

    /**
     * @brief Computes the index in the table for a given hash key
     * @param key Zobrist hash key
     * @return Index in the table
     */
    inline uint32_t index(uint64_t key);

    /**
     * @brief Allocates memory for the transposition table
     * @param size Number of entries to allocate
     */
    void allocate(uint64_t size);

    /**
    * @brief Allocates memory for the transposition table in megabytes
    * @param size_mb Size of the table in megabytes
    */
    void allocateMB(uint64_t size_mb);

    /**
    * @brief Clears all entries in the transposition table
    */
    void clear();

    /**
    * @brief Maximum allowed hash size in MiB
    */
    static constexpr uint64_t MAXHASH_MiB = (1ull << 32) * sizeof(TTEntry) / (1024 * 1024);

   private:
    std::vector<TTEntry> table;
};