#pragma once
#include "../include/chess.hpp"
#include <chrono>

using namespace chess;

/**
 * @brief Recursively counts all legal moves to a given depth
 * 
 * @param board Current board position
 * @param depth Remaining depth to search
 * @return Number of leaf nodes at the specified depth
 */
uint64_t perft(Board& board, int depth);

/**
 * @brief Initializes and runs a perft test from a given position
 * 
 * @param fen FEN string representing the starting position
 * @param depth Maximum depth to search
 * @return The total count of leaf nodes generated at the specified depth.
 */
uint64_t start_perft(const std::string& fen, int depth);