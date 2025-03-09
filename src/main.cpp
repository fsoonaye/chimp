#include "../include/chess.hpp"
#include "uci.h"

int main() {
    UCIEngine uci;
    uci.loop();

    return 0;
}