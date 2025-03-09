#include "../include/chess.hpp"
#include "uci.h"
#include "bench.h"

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "bench")
    {
        run_bench();
        return 0;
    }

    UCIEngine uci;
    uci.loop();

    return 0;
}