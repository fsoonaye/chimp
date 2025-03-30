#pragma once

#include "../include/chess.hpp"
#include "engine.h"

using namespace chess;

class UCIEngine {
   public:
    void loop();
    void print_engine_info();
    void position(std::istringstream& is);
    void go(std::istringstream& is);
    void eval();

    Engine engine;

   private:
};