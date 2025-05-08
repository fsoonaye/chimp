#include "movepicker.h"
#include "evaluate.h"
#include "time.h"
#include "see.h"
#include "engine.h"
#include <algorithm>

using namespace chess;


Move Engine::get_bestmove(int depth) {
    // Initializing variables
    starttime   = std::chrono::high_resolution_clock::now();
    stop_search = false;
    nodes       = 0;
    init_tables();

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

    if (!is_root_node)
    {
        // REPETITION DETECTION
        if (board.isRepetition(1 + is_pv_node))
            return -1 + (nodes & 0x2);

        // 50 MOVE DRAW DETECTION
        if (board.isHalfMoveDraw())
        {
            auto [reason, result] = board.getHalfMoveDrawType();
            if (result == GameResult::DRAW)
                return 0;

            if (result == GameResult::LOSE)
                return mated_in(ply);
        }

        // MATE DISTANCE PRUNING
        alpha = std::max(alpha, mated_in(ply));
        beta  = std::min(beta, mate_in(ply + 1));
        if (alpha >= beta)
            return alpha;
    }

    // CHECK EXTENSION
    if (is_in_check)
        depth++;

    if (depth <= 0)
        return quiescence_search<node>(alpha, beta, ply);

    // TRANSPOSITION TABLE PROBE
    Move     ttmove  = Move::NO_MOVE;
    bool     tthit   = false;
    TTEntry* tte     = tt.probe(board.hash(), ttmove, tthit);
    int      ttscore = tthit ? tte->score : VALUE_NONE;

    // TRANSPOSITION TABLE CUTOFF
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

    if (is_root_node)
        goto moveloop;

    // INTERNAL ITERATIVE REDUCTIONS (IIR)
    if (!tthit)
        depth -= (depth >= 3) + is_pv_node;

    if (depth <= 0)
        return quiescence_search<node>(alpha, beta, ply);

    if (is_in_check || is_pv_node)
        goto moveloop;

    search_info[ply].eval = tthit ? ttscore : evaluate(board);

    // REVERSE FUTILITY PRUNING (RFP)
    if (ttmove != Move::NO_MOVE && !board.isCapture(ttmove))
    {
        int margin = 150 * depth;

        if (search_info[ply].eval >= beta + margin)
            return search_info[ply].eval;
    }

    // NULL MOVE PRUNING (NMP)
    if (depth >= 3 && search_info[ply].eval >= beta)
    {
        board.makeNullMove();
        int nullmove_score = -negamax_search<NON_PV>(-beta, -beta + 1, depth - 3, ply + 1);
        board.unmakeNullMove();

        if (nullmove_score >= beta)
            return nullmove_score >= VALUE_MATE_IN_PLY ? beta : nullmove_score;
    }

moveloop:
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
        bool is_capture = board.isCapture(move);
        int  newdepth   = depth - 1;

        nodes++;
        board.makeMove(move);
        search_info[ply].currentmove = move;

        // LATE MOVE REDUCTION (LMR)
        // clang-format off
        bool do_lmr = depth >= 3
                   && movecount > 2
                   && !is_root_node
                   && !is_in_check
                   && !is_pv_node
                   && !is_capture
                   && move.typeOf() != Move::PROMOTION
                   && move != killer_moves[ply][0]
                   && move != killer_moves[ply][1];
        // clang-format on

        if (do_lmr)
        {
            int reduction = reduction_table[depth][movecount];
            newdepth      = std::max(1, depth - 1 - reduction);

            // Do reduced depth search with null window
            score = -negamax_search<NON_PV>(-alpha - 1, -alpha, newdepth, ply + 1);

            // If score exceeds alpha, do full depth search
            if (score > alpha)
                score = -negamax_search<NON_PV>(-alpha - 1, -alpha, depth - 1, ply + 1);
        }
        else
        {
            // PRINCIPAL VARIATION SEARCH (PVS)
            if (movecount == 1)
                score = -negamax_search<node>(-beta, -alpha, newdepth, ply + 1);
            else
            {
                // Null window search first
                score = -negamax_search<NON_PV>(-alpha - 1, -alpha, newdepth, ply + 1);

                // If score falls within window and we're in a PV node, do full window search
                if (score > alpha && score < beta && is_pv_node)
                    score = -negamax_search<PV>(-beta, -alpha, newdepth, ply + 1);
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

                // PRINCIPAL VARIATION UPDATE
                pv_table[ply][ply] = move;
                for (int nextply = ply + 1; nextply < pv_length[ply + 1]; nextply++)
                    pv_table[ply][nextply] = pv_table[ply + 1][nextply];
                pv_length[ply] = pv_length[ply + 1];
            }
        }

        if (score >= beta)
        {
            // KILLER & HISTORY UPDATES
            if (!is_capture)
                update_quiet_heuristics(move, ply, depth);

            break;
        }
    }

    // CHECKMATE/STALEMATE DETECTION
    if (movecount == 0)
        return board.inCheck() ? mated_in(ply) : 0;

    // TRANSPOSITION TABLE STORE
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
    &&  !is_pv_node
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

    // initializing variables
    Move bestmove = Move::NO_MOVE;
    Move move     = Move::NO_MOVE;

    // generating capture moves
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);

    MovePicker mp(*this, moves, ttmove, ply);
    while ((move = mp.next_move()) != Move::NO_MOVE)
    {
        // STATIC EXCHANGE EVALUATION (SEE) PRUNING
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

    // TRANSPOSITION TABLE STORE
    Bound bound = bestscore >= beta ? BOUND_LOWER : BOUND_UPPER;
    tt.store(board.hash(), DEPTH_QS, bestscore, bestmove, bound);

    return bestscore;
}