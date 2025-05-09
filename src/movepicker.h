#pragma once
#include "chess.hpp"
#include "engine.h"
#include "arrays.h"

/**
 * @enum MoveScore
 * @brief Score categories used for move ordering
 */
enum MoveScore : int16_t {
    SCORE_TT      = 30000,
    SCORE_CAPTURE = 20000,
    SCORE_KILLER1 = 19000,
    SCORE_KILLER2 = 18000,
    SCORE_COUNTER = 17000
};

/**
 * @class MovePicker
 * @brief Efficiently picks moves in optimal order for alpha-beta pruning
 * 
 * Implements a staged move generation and ordering strategy:
 * 1. TT move
 * 2. Captures sorted by MVV-LVA
 * 3. Killer moves
 * 4. Counter moves
 * 5. Regular quiet moves
 */
template<movegen::MoveGenType GenType>
class MovePicker {
   public:
    /**
     * @enum Stage
     * @brief Search stages for the move picking process
     */
    enum class Stage {
        TT,                 // Transposition table move
        GENERATE_CAPTURES,  // Generate and score captures
        GOOD_CAPTURES,      // Good captures (SEE >= 0)
        KILLER1,            // First killer move
        KILLER2,            // Second killer move
        COUNTER,            // Counter move
        GENERATE_QUIET,     // Generate and score quiet moves
        QUIET,              // Quiet moves
        BAD_CAPTURES,       // Bad captures (SEE < 0)
        END                 // No more moves
    };

    /**
     * @brief Constructs a MovePicker for a specific position and move set
     * @param engine The current engine state with board and search context
     * @param ttmove Transposition table move (if available)
     * @param ply Current ply from root position
     */
    MovePicker(const Engine& engine, Move ttmove, int ply) :
        engine(engine),
        ttmove(ttmove),
        ply(ply) {
        is_root_node = (ply == 0);
    }

    /**
     * @brief Returns the next best move according to ordering heuristics
     * @return Next move to try, or NO_MOVE when all moves are exhausted
     */
    Move next_move();

   private:
    /**
     * @brief Finds the index of the move with the highest score starting from a given index
     * @param start_idx Index to start searching from
     * @param moves The movelist to search in
     * @return Index of the best-scored move
     */
    int find_best_from(int start_idx, const Movelist& moves);

    /**
     * @brief Checks if a move exists in the current movelist
     * @param move The move to look for
     * @param moves The movelist to search in
     * @return True if the move is in the movelist, false otherwise
     */
    bool is_in_movelist(Move move, const Movelist& moves) const;

    /**
     * @brief Scores capture moves using MVV-LVA and SEE
     */
    void score_capture_moves();

    /**
     * @brief Scores quiet moves using killer moves, counter moves, and history
     */
    void score_quiet_moves();

    /**
     * @brief Calculates the Most Valuable Victim - Least Valuable Attacker score for a capture
     * @param move The capture move to score
     * @return MVV-LVA score for the move
     */
    int16_t get_mvvlva_score(const Move& move);

    const Engine& engine;
    Movelist      capture_moves;
    Movelist      quiet_moves;

    Move ttmove;
    Move killer1 = Move::NO_MOVE;
    Move killer2 = Move::NO_MOVE;
    Move counter = Move::NO_MOVE;

    int  ply;
    bool is_root_node;

    Stage stage       = Stage::TT;
    int   capture_idx = 0;
    int   quiet_idx   = 0;
};