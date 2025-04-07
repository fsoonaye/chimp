#pragma once
#include "types.h"

/**
 * @brief Calculates the optimal and maximum time to spend on the current move
 * 
 * Heavily inspired from Smallbrain for now. 
 * 
 * @param available_time Time remaining on the clock in milliseconds
 * @param inc Time increment per move in milliseconds
 * @param movestogo Number of moves until the next time control
 * @return Time structure with optimum and maximum time allocations
 */
Time calculate_move_time(int64_t available_time, int inc, int movestogo);