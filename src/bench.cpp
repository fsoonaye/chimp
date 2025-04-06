#include "bench.h"


void bench::run(int depth) {
    uint64_t nodes = 0;

    Limits limits;
    limits.depth = depth;
    limits.nodes = 0;
    limits.time  = Time();

    int i = 1;

    auto t0 = std::chrono::high_resolution_clock::now();

    for (auto& fen : benchfens)
    {
        std::cout << "\nPosition: " << i++ << "/" << benchfens.size() << " " << fen << std::endl;

        Engine engine;
        engine.limits = limits;
        engine.board.setFen(fen);
        engine.get_bestmove(depth);

        nodes += engine.nodes;
    }

    auto t1      = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    auto nps = signed((nodes / (elapsed + 1)) * 1000);

    std::cout << "\n\ninfo string " << elapsed / 1000.0 << " seconds" << std::endl;
    std::cout << nodes << " nodes " << nps << " nps" << std::endl;
}