#pragma once
#include "../include/chess.hpp"

using namespace chess;

class Eval {
   public:
    Eval(const Board& board);

    int evaluate();

    // Getters for individual evaluation components
    int getMaterialScore() const;
    int getPositionalScore() const;

    // Future evaluation components
    int getPawnStructureScore() const;
    int getKingSafetyScore() const;
    int getMobilityScore() const;

   private:
    const Board& board;
    int          phase;  // 0 (endgame) to 256 (middlegame)

    // Calculate the game phase (0-256)
    void calculatePhase();

    // Helper methods for piece-square tables
    int getPieceSquareValue(Piece piece, Square square) const;
    int interpolate(int middlegame, int endgame) const;
};

int getPieceValue(PieceType type);