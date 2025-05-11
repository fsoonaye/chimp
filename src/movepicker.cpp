#include "movepicker.h"
#include "see.h"

template<movegen::MoveGenType GenType>
Move MovePicker<GenType>::next_move() {
    switch (stage)
    {
    case Stage::TT :
        stage = Stage::GENERATE_CAPTURES;
        if (ttmove != Move::NO_MOVE)
            return ttmove;
        [[fallthrough]];

    case Stage::GENERATE_CAPTURES :
        stage = Stage::GOOD_CAPTURES;

        capture_moves.clear();
        movegen::legalmoves<movegen::MoveGenType::CAPTURE>(capture_moves, engine.board);
        score_capture_moves();
        capture_idx = 0;
        [[fallthrough]];

    case Stage::GOOD_CAPTURES :
        while (capture_idx < capture_moves.size())
        {
            int best_idx = find_best_from(capture_idx, capture_moves);
            std::swap(capture_moves[capture_idx], capture_moves[best_idx]);

            // If we've exhausted all good captures, move to next phase
            if (capture_moves[capture_idx].score() < SCORE_CAPTURE)
                break;

            // Skip TT move if we already returned it
            if (capture_moves[capture_idx] != ttmove)
                return capture_moves[capture_idx++];

            capture_idx++;
        }

        if constexpr (GenType == movegen::MoveGenType::CAPTURE)
            stage = Stage::END;
        else
            stage = Stage::GENERATE_QUIET;

        [[fallthrough]];

    case Stage::GENERATE_QUIET :
        if constexpr (GenType == movegen::MoveGenType::ALL)
        {
            stage = Stage::KILLER1;

            quiet_moves.clear();
            movegen::legalmoves<movegen::MoveGenType::QUIET>(quiet_moves, engine.board);
            score_quiet_moves();
            quiet_idx = 0;
            [[fallthrough]];
        }
        else
        {
            stage = Stage::END;
            [[fallthrough]];
        }

    case Stage::KILLER1 :
        stage = Stage::KILLER2;
        if (killer1 != Move::NO_MOVE && killer1 != ttmove)
            return killer1;
        [[fallthrough]];

    case Stage::KILLER2 :
        stage = Stage::COUNTER;
        if (killer2 != Move::NO_MOVE && killer2 != ttmove && killer2 != killer1)
            return killer2;
        [[fallthrough]];

    case Stage::COUNTER :
        stage = Stage::QUIET;
        if (counter != Move::NO_MOVE && counter != ttmove && counter != killer1
            && counter != killer2)
            return counter;
        [[fallthrough]];

    case Stage::QUIET :
        while (quiet_idx < quiet_moves.size())
        {
            int best_idx = find_best_from(quiet_idx, quiet_moves);
            std::swap(quiet_moves[quiet_idx], quiet_moves[best_idx]);

            assert(quiet_moves[quiet_idx].score() < SCORE_CAPTURE);

            // Skip moves we've already returned
            if (quiet_moves[quiet_idx] != ttmove && quiet_moves[quiet_idx] != killer1
                && quiet_moves[quiet_idx] != killer2 && quiet_moves[quiet_idx] != counter)
            {
                return quiet_moves[quiet_idx++];
            }

            quiet_idx++;
        }

        stage = Stage::BAD_CAPTURES;
        [[fallthrough]];

    case Stage::BAD_CAPTURES :
        while (capture_idx < capture_moves.size())
        {
            int best_idx = find_best_from(capture_idx, capture_moves);
            std::swap(capture_moves[capture_idx], capture_moves[best_idx]);

            assert(capture_moves[capture_idx].score() < SCORE_CAPTURE);

            // Skip TT move if we already returned it
            if (capture_moves[capture_idx] != ttmove)
                return capture_moves[capture_idx++];

            capture_idx++;
        }

        return Move::NO_MOVE;

    case Stage::END :
    default :
        return Move::NO_MOVE;
    }
}

template<movegen::MoveGenType GenType>
int MovePicker<GenType>::find_best_from(int start_idx, const Movelist& moves) {
    int best_idx = start_idx;
    for (int i = start_idx + 1; i < moves.size(); i++)
    {
        if (moves[i].score() > moves[best_idx].score())
            best_idx = i;
    }
    return best_idx;
}

template<movegen::MoveGenType GenType>
bool MovePicker<GenType>::is_in_movelist(Move move, const Movelist& moves) const {
    return std::find(moves.begin(), moves.end(), move) != moves.end();
}

template<movegen::MoveGenType GenType>
void MovePicker<GenType>::score_capture_moves() {
    for (auto& move : capture_moves)
    {
        int16_t score;
        if (GenType == movegen::MoveGenType::CAPTURE)
            score = SCORE_CAPTURE + get_mvvlva_score(move);

        else
            score = SEE(engine.board, move, 0) ? SCORE_CAPTURE + get_mvvlva_score(move)
                                               : get_mvvlva_score(move);

        move.setScore(score);
    }
}

template<movegen::MoveGenType GenType>
void MovePicker<GenType>::score_quiet_moves() {
    for (auto& move : quiet_moves)
    {
        int16_t score = 0;

        // Score killer moves
        if (move == engine.killer_moves[ply][0])
        {
            killer1 = move;
            score   = SCORE_KILLER1;
        }
        else if (move == engine.killer_moves[ply][1])
        {
            killer2 = move;
            score   = SCORE_KILLER2;
        }

        // // Score counter moves
        // else if (!is_root_node)
        // {
        //     Move prevmove = engine.search_info[ply - 1].currmove;
        //     if (prevmove != Move::NO_MOVE
        //         && move == engine.counter_moves[prevmove.from().index()][prevmove.to().index()])
        //     {
        //         counter = move;
        //         score   = SCORE_COUNTER;
        //     }
        // }

        // Score remaining quiet moves
        else
        {
            int side     = engine.board.sideToMove();
            int from_idx = move.from().index();
            int to_idx   = move.to().index();
            score        = engine.history_table[side][from_idx][to_idx];
        }

        move.setScore(score);
    }
}

template<movegen::MoveGenType GenType>
int16_t MovePicker<GenType>::get_mvvlva_score(const Move& move) {
    int victim   = engine.board.at<PieceType>(move.to());
    int attacker = engine.board.at<PieceType>(move.from());

    return mvvlva_array[victim][attacker];
}

template class MovePicker<movegen::MoveGenType::ALL>;
template class MovePicker<movegen::MoveGenType::CAPTURE>;
