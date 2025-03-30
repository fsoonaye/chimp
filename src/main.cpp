#include "../include/chess.hpp"
#include "uci.h"
#include "bench.h"
#include "edps.h"

#include <fstream>
#include <string>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "bench")
    {
        bench::run();
        return 0;
    }

    UCIEngine uci;
    uci.loop();

    return 0;
}