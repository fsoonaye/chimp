#pragma once
#include "chess.hpp"
#include "engine.h"
#include "arrays.h"

/**
 * @enum MoveScore
 * @brief Score categories used for move ordering
 */
enum MoveScore : int16_t {
    SCORE_CAPTURE = 20000,
    SCORE_KILLER1 = 19000,
    SCORE_KILLER2 = 18000,
    SCORE_COUNTER = 17000
};

/**
 * @class MovePicker
 * @brief Efficiently picks moves in optimal order for alpha-beta pruning
 * 
 * Implements a move ordering strategy based on:
 * 1. TT move
 * 2. Captures sorted by MVV-LVA
 * 3. Killer moves
 * 4. Regular quiet moves
 */
class MovePicker {
   public:
    /**
     * @brief Constructs a MovePicker for a specific position and move set
     * @param engine The current engine state with board and search context
     * @param moves List of legal moves to be picked
     * @param ttMove Transposition table move (if available)
     * @param ply Current ply from root position
     */
    MovePicker(const Engine& engine, Movelist& moves, Move ttMove, int ply);

    /**
     * @brief Returns the next best move according to ordering heuristics
     * @return Next move to try, or NO_MOVE when all moves are exhausted
     */
    Move next_move();

   private:
    /**
     * @enum Phase
     * @brief Search phases for the move picking process
     */
    enum class Phase {
        TT,
        SCORE,
        CAPTURES,
        KILLER1,
        KILLER2,
        COUNTER,
        QUIET
    };

    /**
     * @brief Finds the index of the move with the highest score starting from a given index
     * @param start_idx Index to start searching from
     * @return Index of the best-scored move
     */
    int find_best_from(int start_idx);

    /**
     * @brief Checks if a move exists in the current movelist
     * @param move The move to look for
     * @return True if the move is in the movelist, false otherwise
     */
    bool is_in_movelist(Move move) const;

    /**
     * @brief Assigns scores to all moves in the movelist for ordering
     */
    void score_moves();

    /**
     * @brief Calculates the Most Valuable Victim - Least Valuable Attacker score for a capture
     * @param move The capture move to score
     * @return MVV-LVA score for the move
     */
    int16_t get_mvvlva_score(const Move& move);

    const Engine& engine;
    Movelist&     movelist;

    Move ttmove;
    Move killer1 = Move::NO_MOVE;
    Move killer2 = Move::NO_MOVE;
    Move counter = Move::NO_MOVE;

    int   ply;
    Phase phase = Phase::TT;
    int   index = 0;
};