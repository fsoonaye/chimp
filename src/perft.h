#pragma once
#include "../include/chess.hpp"

using namespace chess;

uint64_t perft(Board& board, int depth);
uint64_t start_perft(const std::string& fen, int depth);