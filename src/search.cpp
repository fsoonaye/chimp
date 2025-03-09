#include "../include/chess.hpp"
#include "engine.h"

#include <random>

using namespace chess;

Move Engine::get_bestmove(int depth) {
    Movelist movelist;
    movegen::legalmoves(movelist, board);

    return movelist.front();
}