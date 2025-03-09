#pragma once

#include "../include/chess.hpp"

void run_bench() {
    double time       = 1;
    double totalNodes = 1;
    int    nps        = 1;


    std::cout << "info string " << time << " seconds" << std::endl;
    std::cout << totalNodes << " nodes " << nps << " nps" << std::endl;
}
