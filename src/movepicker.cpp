#include "movepicker.h"

MovePicker::MovePicker(const Engine& engine, Movelist& moves, Move ttmove, int ply) :
    engine(engine),
    movelist(moves),
    ttmove(ttmove),
    ply(ply) {}

Move MovePicker::next_move() {
    switch (phase)
    {
    case Phase::TT :
        phase = Phase::SCORE;

        if (ttmove != Move::NO_MOVE && is_in_movelist(ttmove))
            return ttmove;

        [[fallthrough]];

    case Phase::SCORE :
        phase = Phase::CAPTURES;

        // Score all moves before starting to pick them
        score_moves();

        [[fallthrough]];

    case Phase::CAPTURES :
        while (index < movelist.size())
        {
            int best_idx = find_best_from(index);

            // If we've exhausted all captures, move to next phase
            if (movelist[best_idx].score() < SCORE_CAPTURE)
                break;

            std::swap(movelist[index], movelist[best_idx]);

            if (movelist[index] != ttmove)
                return movelist[index++];

            index++;
        }

        phase = Phase::KILLER1;
        [[fallthrough]];

    case Phase::KILLER1 :
        phase = Phase::KILLER2;

        if (killer1 != Move::NO_MOVE && killer1 != ttmove)
            return killer1;

        [[fallthrough]];

    case Phase::KILLER2 :
        phase = Phase::QUIET;

        if (killer2 != Move::NO_MOVE && killer2 != ttmove && killer2 != killer1)
            return killer2;

        [[fallthrough]];

    case Phase::COUNTER :
        phase = Phase::QUIET;

        if (counter != Move::NO_MOVE && counter != ttmove && counter != killer1
            && counter != killer2)
            return counter;

    case Phase::QUIET :
        while (index < movelist.size())
        {
            int best_idx = find_best_from(index);

            std::swap(movelist[index], movelist[best_idx]);

            if (movelist[index] != ttmove && movelist[index] != killer1
                && movelist[index] != killer2)
                return movelist[index++];

            index++;
        }

        return Move::NO_MOVE;

    default :
        return Move::NO_MOVE;
    }
}

int MovePicker::find_best_from(int start_idx) {
    int best_idx = start_idx;
    for (int i = start_idx + 1; i < movelist.size(); i++)
    {
        if (movelist[i].score() > movelist[best_idx].score())
            best_idx = i;
    }
    return best_idx;
}


bool MovePicker::is_in_movelist(Move move) const {
    return std::find(movelist.begin(), movelist.end(), move) != movelist.end();
}

void MovePicker::score_moves() {
    for (auto& move : movelist)
    {
        int16_t score = 0;

        // Score captures using MVV-LVA
        if (engine.board.isCapture(move))
            score = SCORE_CAPTURE + get_mvvlva_score(move);

        // Score killer moves
        else if (move == engine.killer_moves[ply][0])
        {
            killer1 = move;
            score   = SCORE_KILLER1;
        }
        else if (move == engine.killer_moves[ply][1])
        {
            killer2 = move;
            score   = SCORE_KILLER2;
        }

        // Score quiet moves
        else
        {

            // // Score counter moves
            // if (ply > 0)
            // {
            //     Move prevmove = engine.search_info[ply - 1].currmove;
            //     if (prevmove != Move::NO_MOVE
            //         && move == engine.counter_moves[prevmove.from().index()][prevmove.to().index()])
            //     {
            //         counter = move;
            //         score   = SCORE_COUNTER;
            //     }
            // }

            // // Score remaining quiet moves
            // else
            // {
            int side     = engine.board.sideToMove();
            int from_idx = move.from().index();
            int to_idx   = move.to().index();
            score        = engine.history_table[side][from_idx][to_idx];
            //     }
        }

        move.setScore(score);
    }
}

int16_t MovePicker::get_mvvlva_score(const Move& move) {
    int victim   = engine.board.at<PieceType>(move.to());
    int attacker = engine.board.at<PieceType>(move.from());

    return mvvlva_array[victim][attacker];
}
