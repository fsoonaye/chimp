#pragma once
#include "../include/chess.hpp"
#include "evaluate.h"
#include "hash.h"
#include "types.h"

using namespace chess;

class Engine {
   public:
    Move rootSearch(int maxdepth = MAX_PLY);
    int  alphaBeta(int alpha, int beta, int depth, int ply);
    int  quiescence(int alpha, int beta, int ply);
    int  searchPos();
    int  evaluate();

    void storeKillerMove(Move move, int ply);
    void storePvMove(Move move);
    void printPV();
    void orderMoves(Movelist& moves, int ply, Move ttMove);

    bool    isLimit();
    int64_t getTime();

    Board board;

    static const int INF     = 32001;
    static const int MAX_PLY = 64;  // Maximum search depth

    Move   pvMoves[MAX_PLY][MAX_PLY];  // PV table: stores best move sequences
    Move   killerMoves[2][MAX_PLY];
    int    pvLength[MAX_PLY];
    Limits limit;

    void reset() {
        board = Board::fromFen(constants::STARTPOS);
        nodes = 0;
        std::memset(pvMoves, 0, sizeof(pvMoves));
        std::memset(pvLength, 0, sizeof(pvLength));
        std::memset(killerMoves, 0, sizeof(killerMoves));
    }

    void startSearch() {
        t0          = std::chrono::high_resolution_clock::now();
        check_time  = 255;
        nodes       = 0;
        pvLength[0] = 0;
    }

    void
    printSearchInfo(int depth, int score, uint64_t nodes, int64_t timeMs, const std::string& pv) {
        std::cout << "info";
        std::cout << " depth " << depth;
        std::cout << " score cp " << score;  // Centipawn score
        std::cout << " nodes " << nodes;
        std::cout << " time " << timeMs;
        std::cout << " nps " << (timeMs > 0 ? (nodes * 1000) / timeMs : 0);  // Nodes per second
        std::cout << " pv " << pv;
        std::cout << std::endl;
    }

    std::string formatPV() {
        std::string pvStr;
        for (int i = 0; i < pvLength[0]; i++)
            pvStr += uci::moveToUci(pvMoves[0][i]) + " ";

        return pvStr;
    }

   private:
    uint64_t nodes;
    movegen  moveGenerator;

    TT tt{1 << 20};

    std::__1::chrono::high_resolution_clock::time_point t0;

    int check_time;
};

std::uint64_t perft(Board& board, int depth);
void          startPerft(const std::string& fen, int depth);