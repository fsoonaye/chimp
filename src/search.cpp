#include "movepicker.h"
#include "evaluate.h"
#include "time.h"
#include "see.h"
#include "engine.h"
#include <algorithm>

using namespace chess;


Move Engine::get_bestmove(int depth) { return iterative_deepening(depth); }

Move Engine::iterative_deepening(int max_depth) {
    // SEARCH INITIALIZATION
    starttime     = std::chrono::high_resolution_clock::now();
    stop_search   = false;
    nodes         = 0;
    int  score    = -VALUE_INF;
    Move bestmove = Move::NO_MOVE;
    init_tables();

    // STACK INITIALIZATION
    Stack  stack[MAX_PLY + 4] = {};
    Stack* ss                 = stack + 2;
    for (int i = 0; i < MAX_PLY + 1; i++)
        (ss + i)->ply = i;

    // ITERATIVE DEEPENING LOOP
    for (int depth = 1; depth <= max_depth; depth++)
    {
        int prevscore = score;
        score         = aspiration_window_search(depth, prevscore, ss);
        bestmove      = pv_table[0][0];

        // TIME CHECK
        if (time_is_up())
            // current depth has been incompletely searched
            // we print pv for the latest fully searched depth
            break;

        // SEARCH INFO OUTPUT
        print_search_info(depth, score, nodes, get_elapsedtime());
    }

    return bestmove;
}


int Engine::aspiration_window_search(int depth, int prevscore, Stack* ss) {
    // SEARCH INITIALIZATION
    int score = -VALUE_INF;
    int alpha = -VALUE_INF;
    int beta  = VALUE_INF;
    int delta = 50;

    // WINDOW SETUP
    // For deeper searches, use a window around the previous score
    if (depth >= 9)
    {
        alpha = prevscore - delta;
        beta  = prevscore + delta;
    }

    // ASPIRATION WINDOW LOOP
    while (true)
    {
        // WINDOW BOUNDS CHECK
        if (alpha < -3500)
            alpha = -VALUE_INF;

        if (beta > 3500)
            beta = VALUE_INF;

        // SEARCH WITH CURRENT WINDOW
        score = negamax_search<ROOT>(alpha, beta, depth, ss);

        // TIME CHECK
        if (time_is_up())
            return VALUE_NONE;

        // WINDOW ADJUSTMENT
        // If score is outside window, adjust and try again
        if (score <= alpha)
        {
            beta  = (alpha + beta) / 2;
            alpha = std::max(alpha - delta, -VALUE_INF);
            delta += delta / 2;
        }
        else if (score >= beta)
        {
            beta = std::min(beta + delta, VALUE_INF);
            delta += delta / 2;
        }
        else
            break;
    }

    return score;
}

template<NodeType node>
int Engine::negamax_search(int alpha, int beta, int depth, Stack* ss) {
    // TIME CHECK
    if (time_is_up())
        return VALUE_NONE;

    // NODE CLASSIFICATION
    constexpr bool is_root_node = (node == ROOT);
    constexpr bool is_cut_node  = (node == CUT);
    constexpr bool is_pv_node   = !is_cut_node;
    const bool     is_in_check  = board.inCheck();
    bool           improving    = false;

    // PRINCIPAL VARIATION INITIALIZATION
    pv_length[ss->ply] = ss->ply;

    if (!is_root_node)
    {
        // REPETITION DETECTION
        if (board.isRepetition(1 + is_pv_node))
            return -1 + (nodes & 0x2);

        // 50 MOVE DRAW DETECTION
        if (board.isHalfMoveDraw())
        {
            const auto [reason, result] = board.getHalfMoveDrawType();

            if (result == GameResult::DRAW)
                return 0;

            if (result == GameResult::LOSE)
                return mated_in(ss->ply);
        }

        // MATE DISTANCE PRUNING
        alpha = std::max(alpha, mated_in(ss->ply));
        beta  = std::min(beta, mate_in(ss->ply + 1));

        if (alpha >= beta)
            return alpha;
    }

    // CHECK EXTENSION
    if (is_in_check)
        depth++;

    // QUIESCENCE SEARCH
    if (depth <= 0)
        return quiescence_search<node>(alpha, beta, ss);

    // TRANSPOSITION TABLE PROBE
    Move     ttmove  = Move::NO_MOVE;
    bool     tthit   = false;
    TTEntry* tte     = tt.probe(board.hash(), ttmove, tthit);
    int      ttscore = tthit ? tte->score : VALUE_NONE;

    // avoid cutting off the root node
    if (is_root_node)
        goto moveloop;

    // TRANSPOSITION TABLE CUTOFF
    if (is_cut_node && tthit && ttscore != VALUE_NONE && tte->depth >= depth)
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

    // INTERNAL ITERATIVE REDUCTIONS (IIR)
    if (!tthit)
        depth -= (depth >= 3) + is_pv_node;

    // QUIESCENCE SEARCH
    if (depth <= 0)
        return quiescence_search<node>(alpha, beta, ss);

    // avoid pruning too aggressively for in check nodes
    if (is_in_check)
    {
        ss->eval = -VALUE_INF;
        goto moveloop;
    }

    // STATIC BOARD EVALUATION
    ss->eval = tthit ? ttscore : evaluate(board);

    if (ss->ply > 2)
        improving = (ss - 2)->eval != VALUE_NONE && (ss - 2)->eval < ss->eval;

    // avoid pruning too aggressively for pv nodes
    if (is_pv_node)
        goto moveloop;

    // RAZORING
    if (depth < 3 && ss->eval + 150 < alpha)
        return quiescence_search<CUT>(alpha, beta, ss);

    // REVERSE FUTILITY PRUNING (RFP)
    if (ttmove != Move::NO_MOVE && !board.isCapture(ttmove))
    {
        const int margin = 150 * depth;

        if (ss->eval >= beta + margin)
            return ss->eval;
    }

    // NULL MOVE PRUNING (NMP)
    if (depth >= 3 && ss->eval >= beta && ss->currmove != Move::NO_MOVE)
    {
        const int reduction = 5 + std::min(4, depth / 5) + std::min(3, (ss->eval - beta) / 200);
        board.makeNullMove();
        int nullmove_score = -negamax_search<CUT>(-beta, -beta + 1, depth - reduction, ss + 1);
        board.unmakeNullMove();

        if (nullmove_score >= beta)
            return nullmove_score >= VALUE_MATE_IN_PLY ? beta : nullmove_score;
    }

moveloop:
    // MOVE LOOP INITIALIZATION
    int  score;
    int  bestscore  = -VALUE_INF;
    int  movecount  = 0;
    int  quietcount = 0;
    Move bestmove   = Move::NO_MOVE;
    Move move       = Move::NO_MOVE;

    // MOVE GENERATION AND ORDERING
    Movelist moves;
    movegen::legalmoves(moves, board);

    MovePicker mp(*this, moves, ttmove, ss->ply);
    while ((move = mp.next_move()) != Move::NO_MOVE)
    {
        // MOVE CLASSIFICATION
        const bool is_capture   = board.isCapture(move);
        const bool is_promotion = move.typeOf() == Move::PROMOTION;
        const bool gives_check  = board.givesCheck(move) != CheckType::NO_CHECK;
        const bool is_quiet     = !is_capture && !is_promotion && !gives_check;

        // MOVE COUNT UPDATE
        movecount++;
        quietcount += is_quiet;

        // NEW DEPTH COMPUTATION
        const int new_depth = depth - 1;
        const int reduction = get_reduction(depth, movecount, improving, is_pv_node, is_capture);

        if (!is_root_node && bestscore > VALUE_MATED_IN_PLY)
        {
            if (is_quiet)
            {
                // LATE MOVE PRUNING (LMP)
                if (!is_in_check && is_cut_node && depth <= 5 && quietcount > (4 + depth * depth))
                    continue;
            }
        }

        // if (!is_root_node && bestscore > VALUE_MATED_IN_PLY)
        // {

        //     int lmr_depth = new_depth - reduction;

        //     if (gives_check || is_capture)
        //     {
        //         Piece captured_piece = board.at(move.to());

        //         // FUTILITY PRUNING
        //         if (!gives_check && lmr_depth < 7 && !is_in_check)
        //         {
        //             int futility_score = ss->eval + 200 * lmr_depth;

        //             if (futility_score <= alpha)
        //                 continue;
        //         }
        //     }
        // }

        nodes++;
        board.makeMove(move);
        ss->currmove = move;

        // LATE MOVE REDUCTION (LMR)
        // clang-format off
        const bool do_lmr = depth >= 3
                         && movecount > 3 + 2 * is_pv_node
                         && !is_in_check
                         && !is_promotion
                         && move != killer_moves[ss->ply][0]
                         && move != killer_moves[ss->ply][1];
        // clang-format on

        bool do_null_window_search_at_full_depth;
        if (do_lmr)
        {
            // Calculate reduced search depth for LMR
            const int reduced_depth = std::clamp(new_depth - reduction, 1, new_depth + 1);

            score = -negamax_search<CUT>(-alpha - 1, -alpha, reduced_depth, ss + 1);

            // Only do null window search at full depth if the reduced search beats alpha
            // and we actually reduced the depth (to avoid doing the same search twice)
            do_null_window_search_at_full_depth = score > alpha && reduced_depth < new_depth;
        }
        else
            do_null_window_search_at_full_depth = is_cut_node || movecount > 1;

        if (do_null_window_search_at_full_depth)
            score = -negamax_search<CUT>(-alpha - 1, -alpha, new_depth, ss + 1);

        // PRINCIPAL VARIATION SEARCH (PVS)
        // For PV nodes, we do a full window search at full depth in two cases:
        // 1. First move of the node
        // 2. The score falls within the alpha-beta window
        if (is_pv_node && ((score > alpha && score < beta) || movecount == 1))
            score = -negamax_search<PV>(-beta, -alpha, new_depth, ss + 1);

        board.unmakeMove(move);

        // If search has ended prematurely, return immediately without updating anything
        // This ensures we don't store incomplete or incorrect search results
        if (stop_search)
            return VALUE_NONE;
        assert(score != -VALUE_NONE);

        // BEST SCORE AND BOUND UPDATES
        if (score > bestscore)
        {
            bestscore = score;

            if (score > alpha)
            {
                alpha    = score;
                bestmove = move;

                // PRINCIPAL VARIATION UPDATE
                update_pv(move, ss->ply);
            }
        }

        // BETA CUTOFF
        if (score >= beta)
        {
            // KILLER & HISTORY UPDATES
            if (!is_capture)
                update_quiet_heuristics(move, ss->ply, depth);

            break;
        }
    }

    // CHECKMATE/STALEMATE DETECTION
    if (movecount == 0)
        return board.inCheck() ? mated_in(ss->ply) : 0;

    // TRANSPOSITION TABLE STORE
    const Bound bound = bestscore >= beta                         ? BOUND_LOWER
                      : (is_pv_node && bestmove != Move::NO_MOVE) ? BOUND_EXACT
                                                                  : BOUND_UPPER;
    tt.store(board.hash(), depth, bestscore, bestmove, bound);

    return bestscore;
}

template<NodeType node>
int Engine::quiescence_search(int alpha, int beta, Stack* ss) {
    // TIME CHECK
    if (time_is_up())
        return VALUE_NONE;

    // MAX DEPTH CHECK
    if (ss->ply >= MAX_PLY)
        return evaluate(board);

    // NODE CLASSIFICATION
    constexpr bool is_cut_node = (node == CUT);
    constexpr bool is_pv_node  = !is_cut_node;

    // DRAW DETECTION
    if (board.isRepetition(1 + is_pv_node))
        return -1 + (nodes & 0x2);

    // TRANSPOSITION TABLE PROBE
    Move     ttmove  = Move::NO_MOVE;
    bool     tthit   = false;
    TTEntry* tte     = tt.probe(board.hash(), ttmove, tthit);
    int      ttscore = tthit ? tte->score : VALUE_NONE;

    // TRANSPOSITION TABLE CUTOFF
    // clang-format off
    if (tthit
    &&  is_cut_node
    &&  ttscore != VALUE_NONE
    &&   ((tte->bound == BOUND_EXACT)
       || (tte->bound == BOUND_LOWER && ttscore >= beta)
       || (tte->bound == BOUND_UPPER && ttscore <= alpha)))
        return ttscore;
    // clang-format on

    // STAND PAT EVALUATION
    int bestscore = evaluate(board);

    if (bestscore >= beta)
        return bestscore;

    if (bestscore > alpha)
        alpha = bestscore;

    // MOVE LOOP INITIALIZATION
    int  score;
    Move bestmove = Move::NO_MOVE;
    Move move     = Move::NO_MOVE;

    // MOVE GENERATION AND ORDERING
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);

    MovePicker mp(*this, moves, ttmove, ss->ply);
    while ((move = mp.next_move()) != Move::NO_MOVE)
    {
        // STATIC EXCHANGE EVALUATION (SEE) PRUNING
        if (!board.inCheck() && !SEE(board, move, 1))
            continue;

        nodes++;
        board.makeMove(move);
        score = -quiescence_search<node>(-beta, -alpha, ss + 1);
        board.unmakeMove(move);

        // If search has ended prematurely, return immediately without updating anything
        // This ensures we don't store incomplete or incorrect search results
        if (stop_search)
            return VALUE_NONE;
        assert(score != -VALUE_NONE);

        // BEST SCORE AND BOUND UPDATES
        if (score > bestscore)
        {
            bestscore = score;

            if (score > alpha)
            {
                bestmove = move;
                alpha    = score;
            }
        }

        // BETA CUTOFF
        if (score >= beta)
            break;
    }

    // TRANSPOSITION TABLE STORE
    Bound bound = bestscore >= beta ? BOUND_LOWER : BOUND_UPPER;
    tt.store(board.hash(), DEPTH_QS, bestscore, bestmove, bound);

    return bestscore;
}