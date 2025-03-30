#include "../include/chess.hpp"
#include "engine.h"
#include "movepicker.h"
#include "evaluate.h"
#include "time.h"
#include "see.h"

#include <random>

using namespace chess;


Move Engine::get_bestmove(int depth) {
    starttime   = std::chrono::high_resolution_clock::now();
    stop_search = false;
    nodes       = 0;

    return iterative_deepening(depth);
}


Move Engine::iterative_deepening(int max_depth) {
    Move bestmove = Move::NO_MOVE;

    for (int depth = 1; depth <= max_depth; depth++)
    {
        int  bestvalue     = -VALUE_INF;
        Move curr_bestmove = Move::NO_MOVE;
        Move move          = Move::NO_MOVE;

        // probing TT
        uint64_t poskey = board.hash();
        Move     ttmove = Move::NO_MOVE;
        bool     tthit  = false;
        TTEntry* tte    = tt.probe(poskey, ttmove, tthit);

        // TT cutoff
        if (tthit && tte->depth >= depth)
        {
            return tte->move;
        }

        Movelist moves;
        movegen::legalmoves(moves, board);
        MovePicker mp(*this, moves, ttmove);

        while ((move = mp.next_move()) != Move::NO_MOVE)
        {
            board.makeMove(move);
            int value = -absearch(-VALUE_INF, VALUE_INF, depth - 1, 1);
            board.unmakeMove(move);

            if (value > bestvalue)
            {
                curr_bestmove = move;
                bestvalue     = value;
            }
        }

        if (time_is_up())
            break;

        bestmove = curr_bestmove;

        print_search_info(depth, bestvalue, nodes, get_elapsedtime(), bestmove);
    }

    return bestmove;
}


int Engine::absearch(int alpha, int beta, int depth, int ply) {
    nodes++;

    if (time_is_up())
        return 0;

    if (depth == 0)
        return quiescence_search(alpha, beta, depth, ply);

    // draw detection
    if (board.isRepetition() || board.isHalfMoveDraw())
        return 0;

    // probing TT
    uint64_t poskey = board.hash();
    Move     ttmove = Move::NO_MOVE;
    bool     tthit  = false;
    TTEntry* tte    = tt.probe(poskey, ttmove, tthit);

    // TT cutoff
    if (tthit && tte->depth >= depth)
        return tte->score;

    // initializing variables
    int  bestvalue = -VALUE_INF;
    Move bestmove  = Move::NO_MOVE;
    Move move      = Move::NO_MOVE;

    // generating legal moves
    Movelist moves;
    movegen::legalmoves(moves, board);
    MovePicker mp(*this, moves, ttmove);

    while ((move = mp.next_move()) != Move::NO_MOVE)
    {
        board.makeMove(move);
        int value = -absearch(-beta, -alpha, depth - 1, ply + 1);
        board.unmakeMove(move);

        if (value > bestvalue)
        {
            bestvalue = value;
            bestmove  = move;

            if (value > alpha)
                alpha = value;
        }

        if (value >= beta)
            return bestvalue;
    }

    // if no legal moves are generated, it is either a loss or a draw
    if (moves.empty())
        return board.inCheck() ? -VALUE_INF + ply : 0;

    tt.store(poskey, depth, bestvalue, bestmove);

    return bestvalue;
}


int Engine::quiescence_search(int alpha, int beta, int depth, int ply) {
    nodes++;

    if (time_is_up())
        return 0;

    if (ply >= MAX_DEPTH)
        return evaluate(board);

    // draw detection
    if (board.isRepetition() || board.isHalfMoveDraw())
        return 0;

    // probing TT
    uint64_t poskey = board.hash();
    Move     ttmove = Move::NO_MOVE;
    bool     tthit  = false;
    TTEntry* tte    = tt.probe(poskey, ttmove, tthit);

    // TT cutoff
    if (tthit && tte->depth >= depth)
        return tte->score;

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
            return bestscore;
    }

    // if no legal moves are generated, it is either a loss or a draw
    if (moves.empty())
        return board.inCheck() ? -VALUE_INF + ply : 0;

    tt.store(poskey, depth, bestscore, bestmove);

    return bestscore;
}
