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

    return materialScore;
}

int pesto_eval(Board board) {
    int mg_score   = 0;
    int eg_score   = 0;
    int game_phase = 0;

    static constexpr std::array<int, 6> game_phase_inc = {0, 1, 1, 2, 4, 0};

    for (Color color : {Color::WHITE, Color::BLACK})
    {
        int color_sign = (color == Color::WHITE) ? 1 : -1;

        for (PieceType::underlying pt : {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP,
                                         PieceType::ROOK, PieceType::QUEEN, PieceType::KING})
        {
            int      pt_idx = static_cast<int>(pt);
            Bitboard pieces = board.pieces(pt, color);

            while (pieces)
            {
                uint8_t sq     = pieces.pop();
                int     sq_idx = (color == Color::BLACK) ? (sq ^ 56) : sq;

                mg_score += color_sign * pst::mg_table[pt_idx][sq_idx];
                eg_score += color_sign * pst::eg_table[pt_idx][sq_idx];

                // calculate game phase while counting scores
                game_phase += game_phase_inc[pt_idx];
            }
        }
    }

    // 24 corresponds to a full set of pieces on both sides without promotions
    game_phase = std::min(game_phase, 24);

    int eval = (mg_score * game_phase + eg_score * (24 - game_phase)) / 24;

    return (board.sideToMove() == Color::WHITE) ? eval : -eval;
}