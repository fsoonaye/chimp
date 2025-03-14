#include "time.h"


Time calculate_move_time(int64_t available_time, int inc, int movestogo) {
    Time time;
    int  overhead_ms = 10;

    bool tournament_mode = movestogo > 0;

    int mtg = tournament_mode ? movestogo : 40;

    int64_t total = std::max(int64_t(1), available_time + mtg * inc / 2 - mtg * overhead_ms);


    if (tournament_mode)
        time.optimum = std::min(available_time * 0.5, total * 0.9 / mtg);

    else
        // No movestogo specified (increment-only mode)
        time.optimum = (total / 20);

    // Safety check for very low time
    if (time.optimum <= 0)
        time.optimum = static_cast<int64_t>(available_time * 0.02);

    // Set maximum time (cap at 50% of available time)
    time.maximum = static_cast<int64_t>(std::min(2.0 * time.optimum, 0.5 * available_time));

    return time;
}
