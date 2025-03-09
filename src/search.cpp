#include "../include/chess.hpp"
#include "engine.h"

#include <random>

using namespace chess;

Move Engine::get_bestmove(int depth) {
    Movelist movelist;
    movegen::legalmoves(movelist, board);

    static std::mt19937                rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, movelist.size() - 1);

    return movelist.at(dist(rng));
}