#pragma once
#include "../include/chess.hpp"

using namespace chess;

static int SEEvalues[6] = {0, 100, 450, 450, 650, 1250};

bool SEE(Board board, Move move, int treshold);