#pragma once
#include "chess.hpp"
#include "engine.h"
#include "evaluate.h"
#include "perft.h"
#include "types.h"
#include "time.h"
#include <algorithm>

using namespace chess;

/**
 * @class UCIEngine
 * @brief Manages the UCI (Universal Chess Interface) protocol communication
 * 
 * This class handles the interaction between the chess engine and a UCI-compatible
 * chess GUI, parsing commands and sending appropriate responses.
 */
class UCIEngine {
   public:
    /**
     * @brief Processes UCI commands in a loop until exit
     * 
     * Reads commands from standard input, processes them according to the
     * UCI protocol, and writes responses to standard output.
     */
    void loop();

    /**
     * @brief Processes the UCI 'position' command
     * @param is Input stream containing position parameters
     */
    void position(std::istringstream& is);

    /**
     * @brief Processes the UCI 'go' command
     * @param is Input stream containing search parameters
     */
    void go(std::istringstream& is);

    /**
     * @brief Outputs the static evaluation of the current position
     */
    void eval();

    /**
     * @brief Outputs engine identification and options
     * 
     * Sends the engine name, author, and available UCI options to the GUI.
     */
    void print_engine_info();

    void debug(std::istringstream& is);

    Engine engine;
};