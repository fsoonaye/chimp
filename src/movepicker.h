#pragma once

#include "../include/chess.hpp"

#include "engine.h"

#include "arrays.h"


enum MoveScore : int {

    SCORE_CAPTURE = 7'000'000,
    SCORE_KILLER1 = 6'000'000,
    SCORE_KILLER2 = 5'000'000
};


class MovePicker {

   public:
    MovePicker(const Engine& engine, Movelist& moves, Move TTmove, int ply) :
        engine(engine),
        movelist(moves),
        availableTTmove(TTmove),
        ply(ply) {}

    void score() {
        for (int i = 0; i < movelist.size(); i++)
            movelist[i].setScore(get_movescore(movelist[i]));
    }

    int get_movescore(const Move move) {
        // mvvlva
        if (engine.board.isCapture(move))
            return SCORE_CAPTURE + mvvlva(move);

        // killer moves
        if (move == engine.killer_moves[ply][0])
            return SCORE_KILLER1;

        if (move == engine.killer_moves[ply][1])
            return SCORE_KILLER2;

        return 0;
    }

    int mvvlva(Move move) {
        int victim   = engine.board.at<PieceType>(move.to()) + 1;
        int attacker = engine.board.at<PieceType>(move.from()) + 1;
        return mvvlva_array[victim][attacker];
    }

    Move next_move() {
        switch (pick)
        {
        case Pick::TT :
            pick = Pick::SCORE;

            if (availableTTmove != Move::NO_MOVE
                && std::find(movelist.begin(), movelist.end(), availableTTmove) != movelist.end())
            {
                ttmove = availableTTmove;
                return ttmove;
            }

            // continue onto the next case
            [[fallthrough]];

        case Pick::SCORE :
            pick = Pick::CAPTURES;

            score();

            // continue onto the next case
            [[fallthrough]];

        case Pick::CAPTURES :
            while (played < movelist.size())
            {
                int index = played;
                for (int i = 1 + index; i < movelist.size(); i++)
                {
                    if (movelist[i].score() > movelist[index].score())
                        index = i;
                }

                std::swap(movelist[index], movelist[played]);

                if (movelist[played] != ttmove)
                    return movelist[played++];

                played++;
            }

            pick = Pick::QUIET;
            [[fallthrough]];

        case Pick::QUIET :
            while (played < movelist.size())
            {
                int index = played;
                for (int i = 1 + index; i < movelist.size(); i++)
                {
                    if (movelist[i].score() > movelist[index].score())
                        index = i;
                }

                std::swap(movelist[index], movelist[played]);

                if (movelist[played] != ttmove)
                    return movelist[played++];

                played++;
            }

            return Move::NO_MOVE;

        default :
            return Move::NO_MOVE;
        }
    }


   private:
    enum class Pick {
        TT,
        SCORE,
        CAPTURES,
        QUIET
    };
    Pick pick = Pick::TT;

    const Engine& engine;

    int played = 0;

    Movelist& movelist;

    Move availableTTmove = Move::NO_MOVE;
    Move ttmove          = Move::NO_MOVE;
    int  ply;
};