// evaluate.cpp

#include "evaluate.h"
#include "../include/chess.hpp"
#include "data.h"
#include "engine.h"
#include <array>
#include <initializer_list>

using namespace chess;

constexpr std::array<int, 7> pieceValues = {100, 320, 333, 510, 880, 0, 0};

int Engine::evaluate() {
    Eval evaluator(board);
    return evaluator.evaluate();
}

int getPieceValue(PieceType type) { return pieceValues[static_cast<int>(type)]; }

Eval::Eval(const Board& board) :
    board(board) {
    calculatePhase();
}

int Eval::interpolate(int middlegame, int endgame) const {
    return (middlegame * phase + endgame * (256 - phase)) / 256;
}

// clang-format off
void Eval::calculatePhase()
{
    int totalPieces = 
        board.pieces(PieceType::KNIGHT, Color::WHITE).count() +
        board.pieces(PieceType::KNIGHT, Color::BLACK).count() +
        board.pieces(PieceType::BISHOP, Color::WHITE).count() +
        board.pieces(PieceType::BISHOP, Color::BLACK).count() +
        board.pieces(PieceType::ROOK, Color::WHITE).count() +
        board.pieces(PieceType::ROOK, Color::BLACK).count() +
        board.pieces(PieceType::QUEEN, Color::WHITE).count() * 2 +
        board.pieces(PieceType::QUEEN, Color::BLACK).count() * 2;
    
    const int maxPieces = 24;  // Maximum number of piece values (counting queens double)
    phase = std::min(maxPieces, totalPieces) * 256 / maxPieces;
}
// clang-format on

int Eval::evaluate() { return getMaterialScore() + getPositionalScore(); }

int Eval::getMaterialScore() const {
    int mgScore = 0;
    int egScore = 0;

    // Material values for middlegame and endgame
    constexpr std::array<int, 6> mgValues = {100, 320, 330, 500, 900, 0};
    constexpr std::array<int, 6> egValues = {120, 320, 330, 500, 900, 0};

    for (PieceType::underlying type : {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP,
                                       PieceType::ROOK, PieceType::QUEEN, PieceType::KING})
    {
        int whitePieces = board.pieces(type, Color::WHITE).count();
        int blackPieces = board.pieces(type, Color::BLACK).count();

        mgScore += mgValues[static_cast<int>(type)] * (whitePieces - blackPieces);
        egScore += egValues[static_cast<int>(type)] * (whitePieces - blackPieces);
    }

    return interpolate(mgScore, egScore);
}

int Eval::getPositionalScore() const {
    int mgScore = 0;
    int egScore = 0;

    for (Color c : {Color::WHITE, Color::BLACK})
    {
        int colorSign = (c == Color::WHITE) ? 1 : -1;

        for (PieceType::underlying type : {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP,
                                           PieceType::ROOK, PieceType::QUEEN, PieceType::KING})
        {

            // Get bitboard of all pieces of this type and color
            Bitboard pieces = board.pieces(type, c);

            // Process each piece of this type
            while (pieces)
            {
                // Extract least significant bit (first piece)
                int sq = pieces.pop();

                if (c == Color::BLACK)
                {
                    sq = sq ^ 56;  // Flip for black
                }

                // Apply piece-square table values
                int mgPositional = 0;
                int egPositional = 0;

                switch (type)
                {
                case PieceType::PAWN :
                    mgPositional = PST::PAWN[sq];
                    egPositional = PST::PAWN[sq];
                    break;
                case PieceType::KNIGHT :
                    mgPositional = PST::KNIGHT[sq];
                    egPositional = PST::KNIGHT[sq];
                    break;
                case PieceType::BISHOP :
                    mgPositional = PST::BISHOP[sq];
                    egPositional = PST::BISHOP[sq];
                    break;
                case PieceType::ROOK :
                    mgPositional = PST::ROOK[sq];
                    egPositional = PST::ROOK[sq];
                    break;
                case PieceType::QUEEN :
                    mgPositional = PST::QUEEN[sq];
                    egPositional = PST::QUEEN[sq];
                    break;
                case PieceType::KING :
                    mgPositional = PST::KING_MG[sq];
                    egPositional = PST::KING_EG[sq];
                    break;
                }

                mgScore += colorSign * mgPositional;
                egScore += colorSign * egPositional;
            }
        }
    }

    return interpolate(mgScore, egScore);
}
