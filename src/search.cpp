#include "../include/chess.hpp"
#include "engine.h"
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

    init_pv_table();

    return iterative_deepening(depth);
}


Move Engine::iterative_deepening(int max_depth) {
    Move bestmove = Move::NO_MOVE;
    int  score;

    for (int depth = 1; depth <= max_depth; depth++)
    {
        score = negamax_search<PV>(-VALUE_INF, VALUE_INF, depth, 0);

        // current depth has been incompletely searched
        // we return the bestmove and print pv for the latest fully searched depth
        if (time_is_up())
            break;

        bestmove = pv_table[0][0];

        print_search_info(depth, score, nodes, get_elapsedtime());
    }

    return bestmove;
}

template<NodeType nodetype>
int Engine::negamax_search(int alpha, int beta, int depth, int ply) {
    nodes++;

    if (time_is_up())
        return 0;

    // Initializing variables
    bool is_root_node = (ply == 0);
    bool is_pv_node   = (nodetype != NON_PV);

    pv_length[ply] = ply;

    if (depth == 0)
        return quiescence_search(alpha, beta, depth, ply);


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
    uint64_t poskey  = board.hash();
    Move     ttmove  = Move::NO_MOVE;
    bool     tthit   = false;
    TTEntry* tte     = tt.probe(poskey, ttmove, tthit);
    int      ttscore = tthit ? tte->score : VALUE_NONE;

    // TT cutoff
    if (!is_root_node && !is_pv_node && tthit && tte->depth >= depth)
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

    // initializing variables
    int  bestscore = -VALUE_INF;
    Move bestmove  = Move::NO_MOVE;
    Move move      = Move::NO_MOVE;
    int  score;

    // generating legal moves
    Movelist moves;
    movegen::legalmoves(moves, board);

    // check for checkmate or stalemate
    if (moves.empty())
        return board.inCheck() ? mated_in(ply) : 0;

    MovePicker mp(*this, moves, ttmove);
    while ((move = mp.next_move()) != Move::NO_MOVE)
    {
        board.makeMove(move);

        // Principal Variation Search
        if (is_root_node)
            // For the first move → full window search.
            score = -negamax_search<nodetype>(-beta, -alpha, depth - 1, ply + 1);

        else
        {
            // First, search with a null window [-α-1, -α].
            score = -negamax_search<NON_PV>(-alpha - 1, -alpha, depth - 1, ply + 1);

            // If the score ∈ [α, β], it might be better than α, so re-search with full window.
            if (score > alpha && score < beta && is_pv_node)
                score = -negamax_search<PV>(-beta, -alpha, depth - 1, ply + 1);
        }

        board.unmakeMove(move);

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
            // return bestscore;
            break;
    }

    // Store in TT
    Bound bound = bestscore >= beta         ? BOUND_LOWER
                : bestmove != Move::NO_MOVE ? BOUND_EXACT
                                            : BOUND_UPPER;
    tt.store(poskey, depth, bestscore, bestmove, bound);

    return bestscore;
}


int Engine::quiescence_search(int alpha, int beta, int depth, int ply) {
    nodes++;

    if (time_is_up())
        return 0;

    if (ply >= MAX_PLY)
        return evaluate(board);

    // draw detection
    if (board.isRepetition() || board.isHalfMoveDraw())
        return 0;

    // probing TT
    uint64_t poskey  = board.hash();
    Move     ttmove  = Move::NO_MOVE;
    bool     tthit   = false;
    TTEntry* tte     = tt.probe(poskey, ttmove, tthit);
    int      ttscore = tthit ? tte->score : VALUE_NONE;

    // TT cutoff
    if (tthit && tte->depth >= depth)
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

    // initializing variables
    // prematurily evaluating the board to check if we're out of bounds or in need to update alpha
    int bestscore = evaluate(board);
    if (bestscore >= beta)
        return bestscore;
    if (bestscore > alpha)
        alpha = bestscore;

    Move bestmove = Move::NO_MOVE;
    Move move     = Move::NO_MOVE;

    // generating capture moves
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);

    // check for checkmate or stalemate
    if (moves.empty())
        return board.inCheck() ? mated_in(ply) : 0;

    MovePicker mp(*this, moves, ttmove);
    while ((move = mp.next_move()) != Move::NO_MOVE)
    {
        // SEE pruning
        if (!board.inCheck() && !SEE(board, move, 1))
            continue;

        board.makeMove(move);
        int score = -quiescence_search(-beta, -alpha, depth, ply + 1);
        board.unmakeMove(move);

        if (score > bestscore)
        {
            bestscore = score;
            bestmove  = move;

            if (score > alpha)
                alpha = score;
        }

        if (score >= beta)
            // return bestscore;
            break;
    }

    Bound bound = bestscore >= beta         ? BOUND_LOWER
                : bestmove != Move::NO_MOVE ? BOUND_EXACT
                                            : BOUND_UPPER;
    tt.store(poskey, depth, bestscore, bestmove, bound);


    return bestscore;
}