#pragma once
#include "chess.hpp"

using namespace chess;

/**
 * @brief Material values used for Static Exchange Evaluation
 * 
 * Material values for each piece type used specifically in SEE calculations.
 * Values are from weiss for now.
 */
static int SEEvalues[6] = {0, 100, 450, 450, 650, 1250};

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