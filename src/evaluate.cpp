#include "evaluate.h"


int evaluate(Board board) {
    bool white_to_move = board.sideToMove() == Color::WHITE;
    int  mg_score      = 0;
    int  eg_score      = 0;
    int  game_phase    = 0;

    // Material Score
    calculate_material_score(board, mg_score, eg_score, game_phase);

    // Mobility score
    calculate_mobility_score(board, mg_score, eg_score);

    // Tempo bonus for the side to move
    mg_score += white_to_move ? 28 : -28;

    // Phase interpolation
    int eval = (mg_score * game_phase + eg_score * (24 - game_phase)) / 24;

    // half-move clock adjustment
    if (board.halfMoveClock() > 40)
        eval = eval * (100 - board.halfMoveClock()) / 100;

    return (white_to_move) ? eval : -eval;
}

void calculate_material_score(const Board& board, int& mg_score, int& eg_score, int& game_phase) {
    static constexpr std::array<int, 6> game_phase_inc = {0, 1, 1, 2, 4, 0};

    for (Color color : {Color::WHITE, Color::BLACK})
    {
        int color_sign = (color == Color::WHITE) ? 1 : -1;

        for (PieceType::underlying pt : {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP,
                                         PieceType::ROOK, PieceType::QUEEN, PieceType::KING})
        {
            int      pt_idx = static_cast<int>(pt);
            Bitboard pieces = board.pieces(pt, color);

            // bonus for bishop pair
            if (pt == PieceType::BISHOP && pieces.count() >= 2)
            {
                mg_score += color_sign * 30;
                eg_score += color_sign * 50;
            }

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
}
void calculate_mobility_score(const Board& board, int& mg_score, int& eg_score) {
    for (Color color : {Color::WHITE, Color::BLACK})
    {
        int      sign     = (color == Color::WHITE) ? 1 : -1;
        Bitboard friendly = board.us(color);

        // Define area that's not available for mobility
        // - pieces of our own color
        // - enemy pawns that attack squares
        Bitboard mobility_area = ~friendly;
        Bitboard enemy_pawns   = board.pieces(PieceType::PAWN, ~color);

        // Add enemy pawn attack restrictions to the mobility area
        if (color == Color::WHITE)
        {
            mobility_area &= ~(attacks::pawnLeftAttacks<Color::BLACK>(enemy_pawns)
                               | attacks::pawnRightAttacks<Color::BLACK>(enemy_pawns));
        }
        else
        {
            mobility_area &= ~(attacks::pawnLeftAttacks<Color::WHITE>(enemy_pawns)
                               | attacks::pawnRightAttacks<Color::WHITE>(enemy_pawns));
        }

        // Knights
        Bitboard knights = board.pieces(PieceType::KNIGHT, color);
        while (knights)
        {
            Square   sq       = knights.pop();
            Bitboard attacks  = attacks::knight(sq) & mobility_area;
            int      mobility = attacks.count();

            mg_score += sign * eval::knight_mobility_mg[std::min<int>(mobility, 8)];
            eg_score += sign * eval::knight_mobility_eg[std::min<int>(mobility, 8)];
        }

        // Bishops
        Bitboard bishops = board.pieces(PieceType::BISHOP, color);
        while (bishops)
        {
            Square   sq       = bishops.pop();
            Bitboard attacks  = attacks::bishop(sq, board.occ()) & mobility_area;
            int      mobility = attacks.count();

            mg_score += sign * eval::bishop_mobility_mg[std::min<int>(mobility, 13)];
            eg_score += sign * eval::bishop_mobility_eg[std::min<int>(mobility, 13)];
        }

        // Rooks
        Bitboard rooks = board.pieces(PieceType::ROOK, color);
        while (rooks)
        {
            Square   sq       = rooks.pop();
            Bitboard attacks  = attacks::rook(sq, board.occ()) & mobility_area;
            int      mobility = attacks.count();

            mg_score += sign * eval::rook_mobility_mg[std::min<int>(mobility, 14)];
            eg_score += sign * eval::rook_mobility_eg[std::min<int>(mobility, 14)];
        }

        // Queens
        Bitboard queens = board.pieces(PieceType::QUEEN, color);
        while (queens)
        {
            Square   sq       = queens.pop();
            Bitboard attacks  = attacks::queen(sq, board.occ()) & mobility_area;
            int      mobility = attacks.count();

            mg_score += sign * eval::queen_mobility_mg[std::min<int>(mobility, 27)];
            eg_score += sign * eval::queen_mobility_eg[std::min<int>(mobility, 27)];
        }
    }
}