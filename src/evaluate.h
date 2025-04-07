#pragma once
#include "chess.hpp"
#include "arrays.h"

using namespace chess;

/**
 * @brief Evaluates the current board position
 * 
 * @param board Current board position
 * @return Static evaluation score in centipawns
 */
int evaluate(Board board);

void calculate_material_score(const Board& board, int& mg_score, int& eg_score, int& game_phase);
void calculate_mobility_score(const Board& board, int& mg_score, int& eg_score);