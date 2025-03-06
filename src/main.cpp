// main.cpp

#include "../include/chess.hpp"
#include "engine.h"
#include "uci.h"

int main() {
    UCIEngine uci;
    uci.loop();

    // int depth = 7;
    // for (int i = 1; i <= depth; i++)
    //     startPerft(constants::STARTPOS, i);

    // Engine chimp;
    // std::string fen = "r1bqkb1r/ppp2ppp/2n1pn2/3p4/2PP1B2/2N1PN2/PP3PPP/R2QKB1R w KQkq - 0 1";
    // chimp.board.setFen(fen);

    // Eval evaluator(chimp.board);
    // std::cout << evaluator.evaluate() << std::endl;
    // chimp.rootSearch();
    // std::string MATE_IN_5 = "rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 1";
    // std::string BASE = "r1bqkbnr/p1pppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // chimp.board.setFen(MATE_IN_5);

    // chimp.rootSearch();

    return 0;
}