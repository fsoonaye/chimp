
#pragma once
#include "../include/chess.hpp"
#include "arrays.h"

using namespace chess;

/**
 * @brief Evaluates the current board position
 * 
 * @param board Current board position
 * @return Static evaluation score in centipawns
 */
int evaluate(Board board);