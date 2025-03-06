// uci.h
#pragma once

#include "../include/chess.hpp"
#include "engine.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace chess;

class UCIEngine {
   public:
    void loop();
    void position(std::istringstream& is);
    void go(std::istringstream& is);

   private:
    Engine engine;
};
