// uci.cpp

#include "uci.h"
#include "engine.h"
#include <algorithm>

void UCIEngine::position(std::istringstream& is) {
    std::string token, fen;
    is >> token;
    if (token == "startpos")
    {
        fen = constants::STARTPOS;
        is >> token;
    }
    else if (token == "fen")
    {
        while (is >> token && token != "moves")
            fen += token + " ";
    }
    else
        return;

    engine.board.setFen(fen);

    while (is >> token)
        engine.board.makeMove(uci::uciToMove(engine.board, token));
}

void UCIEngine::go(std::istringstream& iss) {
    std::string token;
    int         depth    = engine.MAX_PLY;
    int         movetime = 0;

    engine.limit = Limits();

    // Parse search parameters
    while (iss >> token)
    {
        if (token == "perft")
        {
            iss >> token;
            start_perft(engine.board.getFen(), std::stoi(token));
            return;
        }
        if (token == "depth")
            iss >> depth;
        else if (token == "movestogo")
            iss >> engine.limit.movestogo;
        else if (token == "movetime")
            iss >> movetime;
        else if (token == "wtime")
            iss >> engine.limit.wtime;
        else if (token == "btime")
            iss >> engine.limit.btime;
        else if (token == "winc")
            iss >> engine.limit.winc;
        else if (token == "binc")
            iss >> engine.limit.binc;
        else if (token == "infinite")
            engine.limit.isInfinite = true;
        else if (token == "nodes")
            iss >> engine.limit.nodes;
    }

    // Set time constraints
    if (movetime > 0)
    {
        engine.limit.time.maximum = movetime;
        engine.limit.time.optimum = movetime * 0.95;
    }
    else
    {
        int time =
          engine.board.sideToMove() == Color::WHITE ? engine.limit.wtime : engine.limit.btime;
        int inc = engine.board.sideToMove() == Color::WHITE ? engine.limit.winc : engine.limit.binc;

        if (time > 0)
        {
            int movestogo = engine.limit.movestogo;
            if (movestogo <= 0)
                movestogo = 25;  // Estimate if not provided

            // Allocate time with safety margin
            int baseTime    = time / movestogo;
            int incTime     = inc > 100 ? inc - 100 : inc;
            int timeForMove = baseTime + incTime;

            // Use 1/20th of remaining time as upper bound
            timeForMove = std::min(timeForMove, time / 20);

            // Ensure minimum time usage
            engine.limit.time.maximum = std::max(timeForMove - 50, 50);
            engine.limit.time.optimum = engine.limit.time.maximum * 0.8;
        }
    }

    auto bestMove = engine.get_bestmove(depth);
    std::cout << "bestmove " << uci::moveToUci(bestMove) << std::endl;
}

void UCIEngine::loop() {
    std::string token, input;
    do
    {
        std::getline(std::cin, input);
        std::istringstream is(input);
        token.clear();
        is >> std::skipws >> token;
        if (token == "quit" || token == "stop")
            break;

        if (input == "uci")
        {
            std::cout << "id name CHIMP\n";
            std::cout << "id author Florian\n";
            std::cout << "uciok\n";
        }
        else if (input == "isready")
            std::cout << "readyok\n";

        else if (token == "ucinewgame")
            engine.reset();

        else if (token == "position")
            position(is);

        else if (token == "go")
            go(is);

    } while (token != "quit");
}