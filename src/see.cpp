#include "see.h"

bool SEE(Board board, Move move, int treshold) {
    Square to   = move.to();
    Square from = move.from();

    int swap = SEEvalues[static_cast<int>(board.at<PieceType>(to))] - treshold;

    // If even before subtracting our own piece’s cost, the swap is losing, bail out
    if (swap < 0)
        return false;

    swap -= SEEvalues[static_cast<int>(board.at<PieceType>(from))];

    // If the exchange is still winning after this capture, then accept
    if (swap >= 0)
        return true;

    // Get all pieces' squares
    Bitboard occupied = board.occ();

    // Get all pieces that currently attack the target square
    Bitboard attackers =
      attacks::attackers(board, Color::WHITE, to) | attacks::attackers(board, Color::BLACK, to);

    // For discovered attackers later, define bitboards for sliding pieces
    Bitboard queens             = board.pieces(PieceType::QUEEN);
    Bitboard diagonal_sliders   = board.pieces(PieceType::BISHOP) | queens;
    Bitboard orthogonal_sliders = board.pieces(PieceType::ROOK) | queens;

    // Determine whose turn it is next in the exchange sequence.
    Color side = ~board.at<Piece>(from).color();

    // Loop through potential captures until no attacker is available.
    while (true)
    {
        // Remove any attackers that no longer exist (because their piece was captured)
        attackers &= occupied;

        // Determine the set of attackers for the side to move now.
        Bitboard my_attackers = attackers & board.us(side);

        if (!my_attackers)
            break;

        PieceType::underlying pt = PieceType::PAWN;
        for (auto candidate : {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP,
                               PieceType::ROOK, PieceType::QUEEN, PieceType::KING})
        {
            if (my_attackers & board.pieces(pt))
            {
                pt = candidate;
                break;
            }
        }

        // alternate who is capturing next
        side = ~side;

        // Negamax the swap value
        swap = -swap - 1 - SEEvalues[static_cast<int>(pt)];

        // if the new swap indicates a winning net results, then accept
        if (swap >= 0)
        {

            if (pt == PieceType::KING && (attackers & board.us(side)))
                side = ~side;

            break;
        }

        // remove the captured piece from the board
        occupied ^= (my_attackers & board.pieces(pt)).lsb();


        // update the set of attackers by adding discovered attackers (x-ray)
        if (pt == PieceType::PAWN || pt == PieceType::BISHOP || pt == PieceType::QUEEN)
            attackers |= attacks::bishop(to, occupied) & diagonal_sliders;

        if (pt == PieceType::ROOK || pt == PieceType::QUEEN)
            attackers |= attacks::rook(to, occupied) & orthogonal_sliders;
    }

    // If the final side to move is NOT the same as the color of the original moving piece,
    // then the initiating side wins the exchange.
    return side != board.at<Piece>(from).color();
}