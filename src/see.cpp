#include "see.h"


bool SEE(Board board, Move move, int treshold) {
    Square      to              = move.to();
    Square      from            = move.from();
    const Color initiating_side = board.at<Piece>(from).color();

    int exchange_value = SEE_VALUES[board.at<PieceType>(to)] - treshold;

    // If even before subtracting our own piece's cost, the exchange is losing, then refuse
    if (exchange_value < 0)
        return false;

    exchange_value -= SEE_VALUES[board.at<PieceType>(from)];

    // If the exchange is still winning after this capture, then accept
    if (exchange_value >= 0)
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
    Color side = ~initiating_side;

    // Loop through potential captures until no attacker is available.
    while (true)
    {
        // Remove any attackers that no longer exist (because their piece was captured)
        attackers &= occupied;

        // Determine the set of attackers for the side to move now.
        Bitboard my_attackers = attackers & board.us(side);

        if (my_attackers.empty())
            break;

        // Finding the least valuable attacker
        PieceType pt = PieceType::NONE;
        if (my_attackers & board.pieces(PieceType::PAWN))
            pt = PieceType::PAWN;
        else if (my_attackers & board.pieces(PieceType::KNIGHT))
            pt = PieceType::KNIGHT;
        else if (my_attackers & board.pieces(PieceType::BISHOP))
            pt = PieceType::BISHOP;
        else if (my_attackers & board.pieces(PieceType::ROOK))
            pt = PieceType::ROOK;
        else if (my_attackers & board.pieces(PieceType::QUEEN))
            pt = PieceType::QUEEN;
        else if (my_attackers & board.pieces(PieceType::KING))
            pt = PieceType::KING;
        assert(pt != PieceType::NONE);

        // alternate who is capturing next
        side = ~side;

        // Negamax the exchange value
        exchange_value = -exchange_value - 1 - SEE_VALUES[pt];

        // if the new exchange value indicates a winning net results, then accept
        if (exchange_value >= 0)
        {
            if ((pt == PieceType::KING) && (attackers & board.us(side)))
                side = ~side;

            break;
        }

        // remove the captured piece from the board
        int lsb_index = (my_attackers & board.pieces(pt)).lsb();
        occupied.clear(lsb_index);


        // update the set of attackers by adding discovered attackers (x-ray)
        if (pt == PieceType::PAWN || pt == PieceType::BISHOP || pt == PieceType::QUEEN)
            attackers |= (attacks::bishop(to, occupied) & diagonal_sliders);

        if (pt == PieceType::ROOK || pt == PieceType::QUEEN)
            attackers |= (attacks::rook(to, occupied) & orthogonal_sliders);
    }

    // If the final side to move is NOT the same as the color of the original moving piece,
    // then the initiating side wins the exchange.
    return (side != initiating_side);
}