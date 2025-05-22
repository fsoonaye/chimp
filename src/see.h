#pragma once
#include "chess.hpp"

using namespace chess;


/**
 * @brief Performs Static Exchange Evaluation for a move
 * 
 * Determines if a capture is favorable by simulating the sequence of captures
 * that might follow on the same square. This is used for move ordering and
 * pruning poor captures during search.
 * 
 * @param board Current board position
 * @param move Move to evaluate
 * @param threshold Minimum score for the move to be considered favorable
 * @return True if the move meets or exceeds the threshold score
 */
bool SEE(Board board, Move move, int threshold);

/**
 * @brief Material values used for Static Exchange Evaluation
 * 
 * Material values for each piece type used specifically in SEE calculations.
 * Values are inspired from Smallbrain for now.
 */
static constexpr int16_t SEE_VALUES[7] = {100, 320, 330, 500, 900, 0, 0};