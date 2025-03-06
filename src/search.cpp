// search.cpp

#include "../include/chess.hpp"
#include "engine.h"
#include "evaluate.h"
#include "hash.h"

using namespace chess;

Move Engine::rootSearch(int maxdepth) {
    Move bestMove;
    int  bestScore = -INF;

    startSearch();

    for (int currentDepth = 1; currentDepth <= maxdepth; currentDepth++)
    {
        bestScore = -INF;

        // Probe the transposition table for the root position
        uint64_t    rootHash = board.hash();
        TT::TTEntry entry    = tt.probe(rootHash);
        bool        ttHit    = (entry.hash == rootHash);
        Move        ttMove   = ttHit ? entry.bestMove : Move::NO_MOVE;

        Movelist moves;
        movegen::legalmoves(moves, board);

        orderMoves(moves, 1, ttMove);

        for (const auto& move : moves)
        {
            board.makeMove(move);
            int score = -alphaBeta(-INF, INF, currentDepth, 1);
            board.unmakeMove(move);

            if (score > bestScore)
            {
                bestScore = score;
                bestMove  = move;

                pvMoves[0][0] = move;
                pvLength[0]   = pvLength[1] + 1;
                for (int i = 0; i < pvLength[1]; i++)
                    pvMoves[0][i + 1] = pvMoves[1][i];
            }

            if (isLimit())
                break;
        }

        if (!isLimit())
            tt.store(rootHash, bestScore, TT::Bound::EXACT, currentDepth, bestMove);

        int64_t     timeMs = getTime();
        std::string pv     = formatPV();
        printSearchInfo(currentDepth, bestScore, nodes, timeMs, pv);

        if (isLimit())
            break;
    }

    return bestMove;
}

int Engine::alphaBeta(int alpha, int beta, int depth, int ply) {
    pvLength[ply] = 0;

    if (depth == 0)
    {
        return evaluate();
    }

    if (board.isRepetition() || board.isHalfMoveDraw())
        return 0;

    nodes++;

    if (board.inCheck())
        depth++;

    // Check transposition table
    const uint64_t currentHash = board.hash();
    TT::TTEntry    entry       = tt.probe(currentHash);
    bool           ttHit       = (entry.hash == currentHash);
    Move           ttMove      = ttHit ? entry.bestMove : Move::NO_MOVE;

    // TT cutoff and window adjustment
    if (ttHit && entry.depth >= depth)
    {
        if (entry.bound == TT::Bound::EXACT)
            return entry.score;

        else if (entry.bound == TT::Bound::LOWER && entry.score >= beta)
            return entry.score;

        else if (entry.bound == TT::Bound::UPPER && entry.score <= alpha)
            return entry.score;
    }

    // generate and order moves
    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.empty())
        return board.inCheck() ? -INF + ply : 0;

    orderMoves(moves, ply, ttMove);

    int  bestScore = -INF;
    Move bestMove  = Move::NO_MOVE;
    int  OGalpha   = alpha;

    for (const auto& move : moves)
    {
        board.makeMove(move);
        int score = -alphaBeta(-beta, -alpha, depth - 1, ply + 1);
        board.unmakeMove(move);

        if (isLimit())
            return 0;

        if (score >= beta)
        {
            tt.store(currentHash, score, TT::Bound::LOWER, depth, move);

            if (!board.isCapture(move))
                storeKillerMove(move, ply);

            return beta;
        }

        if (score > bestScore)
        {
            bestScore = score;
            bestMove  = move;

            if (score > alpha)
            {
                alpha = score;

                // Update PV table
                pvMoves[ply][0] = move;
                for (int i = 0; i < pvLength[ply + 1]; i++)
                    pvMoves[ply][i + 1] = pvMoves[ply + 1][i];

                pvLength[ply] = pvLength[ply + 1] + 1;
            }
        }
    }

    // Determine bound and store in TT
    TT::Bound bound = TT::Bound::EXACT;

    if (bestScore <= OGalpha)
        bound = TT::Bound::UPPER;

    else if (bestScore >= beta)
        bound = TT::Bound::LOWER;

    tt.store(currentHash, bestScore, bound, depth, bestMove);

    return bestScore;
}

int Engine::quiescence(int alpha, int beta, int ply) {
    nodes++;
    if ((nodes & 1023) == 0 && isLimit())
        return 0;

    int standPat = evaluate();

    if (standPat >= beta)
        return beta;

    if (standPat > alpha)
        alpha = standPat;

    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);

    // Order captures by MVV-LVA
    std::vector<std::pair<int, size_t>> moveScores;
    moveScores.reserve(moves.size());

    for (int i = 0; i < moves.size(); i++)
    {
        const auto& move      = moves[i];
        Piece       captured  = board.at(move.to());
        Piece       aggressor = board.at(move.from());

        int captureValue   = getPieceValue(captured.type());
        int aggressorValue = getPieceValue(aggressor.type());

        // MVV-LVA score
        int score = captureValue * 10 - aggressorValue;

        // Promotions are valuable too
        if (move.typeOf() == Move::PROMOTION)
        {
            score += getPieceValue(move.promotionType());
        }

        moveScores.emplace_back(score, i);
    }


    std::sort(moveScores.begin(), moveScores.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    for (const auto& [moveScore, index] : moveScores)
    {
        const auto& move = moves[index];

        // SEE
        if (moveScore < 0 && standPat + moveScore + 100 < alpha)
            continue;

        board.makeMove(move);
        int score = -quiescence(-beta, -alpha, ply + 1);
        board.unmakeMove(move);

        if (score >= beta)
            return beta;

        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

bool Engine::isLimit() {
    if (limit.nodes != 0 && nodes >= limit.nodes)
        return true;

    // Check time more frequently (every 256 nodes)
    if ((nodes & 255) == 0 && limit.time.maximum)
    {
        auto elapsed = getTime();
        if (elapsed >= limit.time.maximum)
        {
            // DEBUGGING
            // std::cerr << "Elapsed: " << elapsed << "ms / Limit: " << limit.time.maximum << std::endl;
            return true;
        }
    }

    return false;
}

int64_t Engine::getTime() {
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
}

void Engine::printPV() {
    for (int i = 0; i < pvLength[0]; i++)
    {
        std::cout << uci::moveToUci(pvMoves[0][i]) << " ";
    }
}

// Store killer moves for move ordering
void Engine::storeKillerMove(Move move, int ply) {
    if (ply >= MAX_PLY)
        return;

    if (killerMoves[0][ply] != move)
    {
        killerMoves[1][ply] = killerMoves[0][ply];
        killerMoves[0][ply] = move;
    }
}