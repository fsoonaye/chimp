// uci.cpp

#include <algorithm>
#include "uci.h"
#include "engine.h"
#include "types.h"
#include "perft.h"

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
    int         depth    = MAX_DEPTH;
    int         movetime = 0;

    engine.limits = Limits();

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
            iss >> engine.limits.movestogo;
        else if (token == "movetime")
            iss >> movetime;
        else if (token == "wtime")
            iss >> engine.limits.wtime;
        else if (token == "btime")
            iss >> engine.limits.btime;
        else if (token == "winc")
            iss >> engine.limits.winc;
        else if (token == "binc")
            iss >> engine.limits.binc;
        else if (token == "infinite")
            engine.limits.isInfinite = true;
        else if (token == "nodes")
            iss >> engine.limits.nodes;
    }


    auto bestmove = engine.get_bestmove(depth);
    std::cout << "bestmove " << uci::moveToUci(bestmove) << std::endl;
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
            std::cout << "option name Hash type spin default 1 min 1 max 1\n";
            std::cout << "option name Threads type spin default 1 min 1 max 1\n";
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