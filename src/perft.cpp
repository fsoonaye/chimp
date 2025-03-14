#include "perft.h"

std::uint64_t perft(Board& board, int depth) {
    Movelist moves;
    movegen::legalmoves(moves, board);

    if (depth == 1)
        return moves.size();

    uint64_t nodes = 0;

    for (int i = 0; i < moves.size(); i++)
    {
        const auto move = moves[i];
        board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.unmakeMove(move);
    }

    return nodes;
}

uint64_t start_perft(const std::string& fen, int depth) {
    Board    board   = Board::fromFen(fen);
    auto     t0      = std::chrono::high_resolution_clock::now();
    uint64_t nodes   = perft(board, depth);
    auto     t1      = std::chrono::high_resolution_clock::now();
    auto     elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    uint64_t nps     = (nodes * 1000) / (elapsed + 1);
    std::cout << "\ntime: " << elapsed << "ms" << std::endl;
    std::cout << "Nodes:" << nodes << " nps " << nps << std::endl;

    return nodes;
}