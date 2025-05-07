#include "movepicker.h"
#include "evaluate.h"
#include "time.h"
#include "see.h"

using namespace chess;


Move Engine::get_bestmove(int depth) {
    // Initializing variables
    starttime   = std::chrono::high_resolution_clock::now();
    stop_search = false;
    nodes       = 0;
    init_heuristic_tables();

    return iterative_deepening(depth);
}


Move Engine::iterative_deepening(int max_depth) {
    Move bestmove = Move::NO_MOVE;
    int  score;

    for (int depth = 1; depth <= max_depth; depth++)
    {
        score    = negamax_search<PV>(-VALUE_INF, VALUE_INF, depth, 0);
        bestmove = pv_table[0][0];

        if (time_is_up())
            // current depth has been incompletely searched
            // we print pv for the latest fully searched depth
            break;

        print_search_info(depth, score, nodes, get_elapsedtime());
    }

    return bestmove;
}

template<Node node>
int Engine::negamax_search(int alpha, int beta, int depth, int ply) {
    if (time_is_up())
        return VALUE_NONE;

    // Initializing variables
    bool is_root_node = (ply == 0);
    bool is_pv_node   = (node != NON_PV);
    bool is_in_check  = board.inCheck();

    pv_length[ply] = ply;

    if (depth <= 0)
        return quiescence_search<node>(alpha, beta, ply);


    if (!is_root_node)
    {
        // Draw or repetition detection
        if (board.isRepetition() || board.isHalfMoveDraw())
            return 0;

        // Mate distance pruning
        alpha = std::max(alpha, mated_in(ply));
        beta  = std::min(beta, mate_in(ply + 1));
        if (alpha >= beta)
            return alpha;
    }

    // probing TT
    Move     ttmove  = Move::NO_MOVE;
    bool     tthit   = false;
    TTEntry* tte     = tt.probe(board.hash(), ttmove, tthit);
    int      ttscore = tthit ? tte->score : VALUE_NONE;

    // TT cutoff
    if (!is_root_node && !is_pv_node && tthit && ttscore != VALUE_NONE && tte->depth >= depth)
    {
        if (tte->bound == BOUND_EXACT)
            return ttscore;

        else if (tte->bound == BOUND_LOWER)
            alpha = std::max(alpha, ttscore);

        else if (tte->bound == BOUND_UPPER)
            beta = std::min(beta, ttscore);

        if (alpha >= beta)
            return ttscore;
    }

    int static_eval = tthit ? ttscore : evaluate(board);

    // Reverse Futility Pruning (RFP)
    if (!is_in_check && !is_pv_node && ttmove != Move::NO_MOVE && !board.isCapture(ttmove))
    {
        int margin = 150 * depth;

        if (static_eval >= beta + margin)
            return static_eval;
    }

    // Null Move Pruning (NMP)
    if (!is_in_check && !is_pv_node && depth >= 3 && static_eval >= beta)
    {
        board.makeNullMove();
        int nullmove_score = -negamax_search<NON_PV>(-beta, -beta + 1, depth - 3, ply + 1);
        board.unmakeNullMove();

        if (nullmove_score >= beta)
            return nullmove_score >= VALUE_MATE_IN_PLY ? beta : nullmove_score;
    }

    // initializing variables
    int  bestscore = -VALUE_INF;
    Move bestmove  = Move::NO_MOVE;
    Move move      = Move::NO_MOVE;
    int  movecount = 0;
    int  score;

    // generating legal moves
    Movelist moves;
    movegen::legalmoves(moves, board);

    MovePicker mp(*this, moves, ttmove, ply);
    while ((move = mp.next_move()) != Move::NO_MOVE)
    {
        movecount++;

        bool go_full_depth = true;
        bool is_capture    = board.isCapture(move);
        int  newdepth      = depth - 1;

        nodes++;
        board.makeMove(move);

        // clang-format off
        if (depth >= 3 && movecount > 3 &&
            !is_root_node && !is_in_check && !is_pv_node && 
            move.typeOf() != Move::PROMOTION && !is_capture)
        {  // clang-format on
            int reduction = 1 + std::log(depth) * std::log(movecount) / 3;
            newdepth      = std::max(1, depth - 1 - reduction);
            go_full_depth = false;
        }

        // Search with reduced depth if LMR applies
        if (!go_full_depth)
        {
            score = -negamax_search<NON_PV>(-alpha - 1, -alpha, newdepth, ply + 1);

            if (score > alpha)
                go_full_depth = true;
        }
        else
        {
            // Principal Variation Search
            if (movecount == 1)
                // For the first move → full window search.
                score = -negamax_search<node>(-beta, -alpha, depth - 1, ply + 1);

            else
            {
                // First, search with a null window [-α-1, -α].
                score = -negamax_search<NON_PV>(-alpha - 1, -alpha, depth - 1, ply + 1);

                // If the score ∈ [α, β], it might be better than α, so re-search with full window.
                if (score > alpha && score < beta && is_pv_node)
                    score = -negamax_search<PV>(-beta, -alpha, depth - 1, ply + 1);
            }
        }

        board.unmakeMove(move);

        // Early exit if search should be stopped: we should not be updating bounds or bestscore
        if (stop_search)
            return VALUE_NONE;
        assert(score != -VALUE_NONE);

        if (score > bestscore)
        {
            bestscore = score;

            if (score > alpha)
            {
                alpha    = score;
                bestmove = move;

                // Update principal variation:
                pv_table[ply][ply] = move;

                // copying pv from the ply just after
                for (int nextply = ply + 1; nextply < pv_length[ply + 1]; nextply++)
                    pv_table[ply][nextply] = pv_table[ply + 1][nextply];

                // update current pv length from the ply just after
                pv_length[ply] = pv_length[ply + 1];
            }
        }

        if (score >= beta)
        {
            // Store killer moves
            if (move != killer_moves[ply][0] && !is_capture)
            {
                // Shift the previous killer and store the new one
                killer_moves[ply][1] = killer_moves[ply][0];
                killer_moves[ply][0] = move;
            }

            break;
        }
    }

    // check for checkmate or stalemate
    if (movecount == 0)
        return is_in_check ? mated_in(ply) : 0;

    // Store in TT
    Bound bound = bestscore >= beta                         ? BOUND_LOWER
                : (is_pv_node && bestmove != Move::NO_MOVE) ? BOUND_EXACT
                                                            : BOUND_UPPER;
    tt.store(board.hash(), depth, bestscore, bestmove, bound);

    return bestscore;
}

template<Node node>
int Engine::quiescence_search(int alpha, int beta, int ply) {
    if (time_is_up())
        return VALUE_NONE;

    if (ply >= MAX_PLY)
        return evaluate(board);

    constexpr bool is_pv_node = node == PV;

    // draw detection
    if (board.isRepetition() || board.isHalfMoveDraw())
        return 0;

    // probing TT
    Move     ttmove  = Move::NO_MOVE;
    bool     tthit   = false;
    TTEntry* tte     = tt.probe(board.hash(), ttmove, tthit);
    int      ttscore = tthit ? tte->score : VALUE_NONE;

    // TT cutoff
    // clang-format off
    if (tthit &&
        !is_pv_node &&
        ttscore != VALUE_NONE &&
        ((tte->bound == BOUND_EXACT) ||
         (tte->bound == BOUND_LOWER && ttscore >= beta) ||
         (tte->bound == BOUND_UPPER && ttscore <= alpha)))
        return ttscore;
    // clang-format on

    // prematurily evaluating the board to check if we're out of bounds or in need to update alpha
    int bestscore = evaluate(board);
    if (bestscore >= beta)
        return bestscore;
    if (bestscore > alpha)
        alpha = bestscore;

    // initializing variables
    Move bestmove = Move::NO_MOVE;
    Move move     = Move::NO_MOVE;

    // generating capture moves
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);

    MovePicker mp(*this, moves, ttmove, ply);
    while ((move = mp.next_move()) != Move::NO_MOVE)
    {
        // SEE pruning
        if (!board.inCheck() && !SEE(board, move, 1))
            continue;

        nodes++;
        board.makeMove(move);
        int score = -quiescence_search<node>(-beta, -alpha, ply + 1);
        board.unmakeMove(move);

        // Early exit if search should be stopped: we should not be updating bounds or bestscore
        if (stop_search)
            return VALUE_NONE;
        assert(score != -VALUE_NONE);

        if (score > bestscore)
        {
            bestscore = score;

            if (score > alpha)
            {
                bestmove = move;
                alpha    = score;
            }
        }

        if (score >= beta)
            break;
    }

    // Store in TT
    Bound bound = bestscore >= beta ? BOUND_LOWER : BOUND_UPPER;
    tt.store(board.hash(), DEPTH_QS, bestscore, bestmove, bound);


    return bestscore;
}