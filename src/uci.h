#pragma once

#include "../include/chess.hpp"
#include "engine.h"

using namespace chess;

class UCIEngine {
   public:
    void loop();
    void position(std::istringstream& is);
    void go(std::istringstream& is);

   private:
    Engine engine;
};