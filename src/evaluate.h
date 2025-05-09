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

/**
 * @brief Calculates material and piece-square table scores
 * 
 * Updates both middlegame and endgame scores based on piece values and their positions.
 * Also tracks the game phase which is used to interpolate between middlegame and endgame scores.
 * 
 * @param board Current board position
 * @param mg_score Reference to middlegame score to be updated
 * @param eg_score Reference to endgame score to be updated
 * @param game_phase Reference to game phase counter to be updated
 */
void calculate_material_score(const Board& board, int& mg_score, int& eg_score, int& game_phase);

/**
 * @brief Calculates mobility scores for pieces
 * 
 * Updates both middlegame and endgame scores based on piece mobility.
 * Evaluates piece mobility by counting the number of legal moves each piece can make,
 * considering restrictions from friendly pieces and enemy pawn attacks.
 * 
 * @param board Current board position
 * @param mg_score Reference to middlegame score to be updated
 * @param eg_score Reference to endgame score to be updated
 */
void calculate_mobility_score(const Board& board, int& mg_score, int& eg_score);