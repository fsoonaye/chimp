#include "../include/chess.hpp"
#include "evaluate.h"

int evaluate(Board board) {
    constexpr std::array<int, 6> ptValues = {120, 320, 330, 500, 900};

    int materialScore = 0;

    for (PieceType::underlying type :
         {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN})
    {
        int whitePieces = board.pieces(type, Color::WHITE).count();
        int blackPieces = board.pieces(type, Color::BLACK).count();

        materialScore += ptValues[static_cast<int>(type)] * (whitePieces - blackPieces);
    }

    return board.sideToMove() == Color::WHITE ? materialScore : -materialScore;
}