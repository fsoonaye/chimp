// search.cpp

#include "engine.h"

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

void startPerft(const std::string& fen, int depth) {
    Board board = Board::fromFen(fen);
    auto  t0    = std::chrono::high_resolution_clock::now();
    int   nodes = perft(board, depth);
    auto  t1    = std::chrono::high_resolution_clock::now();
    auto  ms    = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "\ntime: " << ms << "ms" << std::endl;
    std::cout << "Nodes:" << nodes << " nps " << ((nodes * 1000) / (ms + 1)) << std::endl;
}