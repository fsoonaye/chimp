#include "../include/chess.hpp"
#include "engine.h"

void Engine::orderMoves(Movelist& moves, int ply, Move ttMove) {
    Move pvMove = (ply < MAX_PLY && pvLength[ply] > 0) ? pvMoves[ply][0] : Move();

    std::vector<std::pair<int, size_t>> moveScores;
    moveScores.reserve(moves.size());

    for (int i = 0; i < moves.size(); ++i)
    {
        const auto& move  = moves[i];
        int         score = 0;

        // TT move has the highest priority
        if (move == ttMove)
        {
            score = 30000;
        }

        // PV move has the next highest priority
        if (move == pvMove)
            score = 20000;

        // Capture moves ordered by MVV-LVA
        else if (board.isCapture(move))
        {
            Piece captured  = board.at(move.to());
            Piece aggressor = board.at(move.from());
            // should be using something else here
            int captureValue   = getPieceValue(captured.type());
            int aggressorValue = getPieceValue(aggressor.type());

            score = 10000 + (captureValue * 10) - aggressorValue;
        }
        // Killer moves
        else if (move == killerMoves[0][ply])
            score = 9000;

        else if (move == killerMoves[1][ply])
            score = 8000;

        moveScores.emplace_back(score, i);
    }

    // Sort moves based on scores in descending order
    std::sort(moveScores.begin(), moveScores.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Create a new ordered Movelist
    Movelist orderedMoves;
    for (const auto& [score, index] : moveScores)
    {
        orderedMoves.add(moves[index]);
    }

    moves = orderedMoves;
}

// Helper function to get piece values for MVV-LVA
// int Engine::getPieceValue(int pieceType) {
//     static const int values[] = {0, 100, 320, 330, 500, 900, 0}; // None, Pawn, Knight, Bishop, Rook, Queen, King
//     return values[pieceType];
// }