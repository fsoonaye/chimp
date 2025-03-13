#include "../include/chess.hpp"
#include "engine.h"
#include "evaluate.h"
#include "time.h"

#include <random>

using namespace chess;

Move Engine::get_randommove() {
    Movelist movelist;
    movegen::legalmoves(movelist, board);

    static std::mt19937                rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, movelist.size() - 1);

    return movelist.at(dist(rng));
}

Move Engine::get_bestmove(int depth) {
    int64_t available_time = board.sideToMove() == Color::WHITE ? limits.wtime : limits.btime;
    int64_t inc            = board.sideToMove() == Color::WHITE ? limits.winc : limits.binc;

    if (available_time > 0)
        limits.time = calculate_time(available_time, inc, limits.movestogo);

    return iterative_deepening(depth);
}


int Engine::absearch(int alpha, int beta, int depth, int ply) {
    nodes++;

    if (time_is_up())
        return 0;

    if (depth == 0)
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
    int  bestvalue = -VALUE_INF;
    Move bestmove  = Move::NO_MOVE;

    // generating legal moves
    Movelist moves;
    movegen::legalmoves(moves, board);

    for (const auto& move : moves)
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

Move Engine::iterative_deepening(int max_depth) {
    Move bestmove = Move::NO_MOVE;

    nodes = 0;

    starttime = std::chrono::high_resolution_clock::now();

    for (int depth = 1; depth <= max_depth; depth++)
    {
        Move curr_bestmove = Move::NO_MOVE;
        int  bestvalue     = -VALUE_INF;

        Movelist moves;
        movegen::legalmoves(moves, board);

        for (const auto& move : moves)
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

        bestmove = curr_bestmove;

        if (time_is_up())
            break;
    }

    return bestmove;
}

bool Engine::time_is_up() {
    int64_t elapsed = get_elapsedtime();
    if (limits.time.optimum != 0)
        return elapsed >= limits.time.optimum;

    return false;
}