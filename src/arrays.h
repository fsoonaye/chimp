#pragma once
#include "chess.hpp"
#include <array>
// clang-format off

using namespace chess;

// None is for handling en passant (pawn takes pawn is 0 anyway)
// value = (Victim * 6) - Attacker + 1
static constexpr int16_t mvvlva_array[7][7] = {
    // Attackers:
    // P   N   B   R   Q   K   NONE   Victims:
    {  5,  4,  3,  2,  1,  0,  0  },  // P
    { 11, 10,  9,  8,  7,  6,  0  },  // N
    { 17, 16, 15, 14, 13, 12,  0  },  // B
    { 23, 22, 21, 20, 19, 18,  0  },  // R
    { 29, 28, 27, 26, 25, 24,  0  },  // Q
    { 35, 34, 33, 32, 31, 30,  0  },  // K
    {  0,  0,  0,  0,  0,  0,  0  }   // NONE
};


// values are from https://hxim.github.io/Stockfish-Evaluation-Guide/
namespace pst {
    // Static piece values for middlegame
    static constexpr std::array<int, 6> mg_value = {
        124,   // PAWN
        781,   // KNIGHT
        825,   // BISHOP
        1276,  // ROOK
        2538,  // QUEEN
        0      // KING
    };

    // Static piece values for endgame
    static constexpr std::array<int, 6> eg_value = {
        206,   // PAWN
        854,   // KNIGHT
        915,   // BISHOP
        1380,  // ROOK
        2682,  // QUEEN
        0      // KING
    };

    // Piece-square tables for middlegame
    static constexpr std::array<int, 64> mg_pawn_table = {
        0,   0,   0,   0,   0,   0,   0,   0,
        3,   3,  10,  19,  16,  19,   7,  -5,
       -9, -15,  11,  15,  32,  22,   5, -22,
       -4, -23,   6,  20,  40,  17,   4,  -8,
       13,   0, -13,   1,  11,  -2, -13,   5,
        5, -12,  -7,  22,  -8,  -5, -15,  -8,
       -7,   7,  -3, -13,   5, -16,  10,  -8,
        0,   0,   0,   0,   0,   0,   0,   0
    };

    static constexpr std::array<int, 64> mg_knight_table = {
      -175, -92, -74, -73, -73, -74, -92, -175,
       -77, -41, -27, -15, -15, -27, -41,  -77,
       -61, -17,   6,  12,  12,   6, -17,  -61,
       -35,   8,  40,  49,  49,  40,   8,  -35,
       -34,  13,  44,  51,  51,  44,  13,  -34,
        -9,  22,  58,  53,  53,  58,  22,   -9,
       -67, -27,   4,  37,  37,   4, -27,  -67,
      -201, -83, -56, -26, -26, -56, -83, -201
    };

    static constexpr std::array<int, 64> mg_bishop_table = {
       -53,  -5,  -8, -23, -23,  -8,  -5,  -53,
       -15,   8,  19,   4,   4,  19,   8,  -15,
        -7,  21,  -5,  17,  17,  -5,  21,   -7,
        -5,  11,  25,  39,  39,  25,  11,   -5,
       -12,  29,  22,  31,  31,  22,  29,  -12,
       -16,   6,   1,  11,  11,   1,   6,  -16,
       -17, -14,   5,   0,   0,   5, -14,  -17,
       -48,   1, -14, -23, -23, -14,   1,  -48
    };

    static constexpr std::array<int, 64> mg_rook_table = {
       -31, -20, -14,  -5,  -5, -14, -20,  -31,
       -21, -13,  -8,   6,   6,  -8, -13,  -21,
       -25, -11,  -1,   3,   3,  -1, -11,  -25,
       -13,  -5,  -4,  -6,  -6,  -4,  -5,  -13,
       -27, -15,  -4,   3,   3,  -4, -15,  -27,
       -22,  -2,   6,  12,  12,   6,  -2,  -22,
        -2,  12,  16,  18,  18,  16,  12,   -2,
       -17, -19,  -1,   9,   9,  -1, -19,  -17
    };

    static constexpr std::array<int, 64> mg_queen_table = {
         3,  -5,  -5,   4,   4,  -5,  -5,   3,
        -3,   5,   8,  12,  12,   8,   5,  -3,
        -3,   6,  13,   7,   7,  13,   6,  -3,
         4,   5,   9,   8,   8,   9,   5,   4,
         0,  14,  12,   5,   5,  12,  14,   0,
        -4,  10,   6,   8,   8,   6,  10,  -4,
        -5,   6,  10,   8,   8,  10,   6,  -5,
        -2,  -2,   1,  -2,  -2,   1,  -2,  -2
    };

    static constexpr std::array<int, 64> mg_king_table = {
       271, 327, 271, 198, 198, 271, 327, 271,
       278, 303, 234, 179, 179, 234, 303, 278,
       195, 258, 169, 120, 120, 169, 258, 195,
       164, 190, 138,  98,  98, 138, 190, 164,
       154, 179, 105,  70,  70, 105, 179, 154,
       123, 145,  81,  31,  31,  81, 145, 123,
        88, 120,  65,  33,  33,  65, 120,  88,
        59,  89,  45,  -1,  -1,  45,  89,  59
    };

    // Piece-square tables for endgame
    static constexpr std::array<int, 64> eg_pawn_table = {
        0,   0,   0,   0,   0,   0,   0,   0,
      -10,  -6,  10,   0,  14,   7,  -5, -19,
      -10, -10, -10,   4,   4,   3,  -6,  -4,
        6,  -2,  -8,  -4, -13, -12, -10,  -9,
       10,   5,   4,  -5,  -5,  -5,  14,   9,
       28,  20,  21,  28,  30,   7,   6,  13,
        0, -11,  12,  21,  25,  19,   4,   7,
        0,   0,   0,   0,   0,   0,   0,   0
    };

    static constexpr std::array<int, 64> eg_knight_table = {
       -96, -65, -49, -21, -21, -49, -65,  -96,
       -67, -54, -18,   8,   8, -18, -54,  -67,
       -40, -27,  -8,  29,  29,  -8, -27,  -40,
       -35,  -2,  13,  28,  28,  13,  -2,  -35,
       -45, -16,   9,  39,  39,   9, -16,  -45,
       -51, -44, -16,  17,  17, -16, -44,  -51,
       -69, -50, -51,  12,  12, -51, -50,  -69,
      -100, -88, -56, -17, -17, -56, -88, -100
    };

    static constexpr std::array<int, 64> eg_bishop_table = {
       -57, -30, -37, -12, -12, -37, -30,  -57,
       -37, -13, -17,   1,   1, -17, -13,  -37,
       -16,  -1,  -2,  10,  10,  -2,  -1,  -16,
       -20,  -6,   0,  17,  17,   0,  -6,  -20,
       -17,  -1, -14,  15,  15, -14,  -1,  -17,
       -30,   6,   4,   6,   6,   4,   6,  -30,
       -31, -20,  -1,   1,   1,  -1, -20,  -31,
       -46, -42, -37, -24, -24, -37, -42,  -46
    };

    static constexpr std::array<int, 64> eg_rook_table = {
        -9, -13, -10,  -9,  -9, -10, -13,   -9,
       -12,  -9,  -1,  -2,  -2,  -1,  -9,  -12,
         6,  -8,  -2,  -6,  -6,  -2,  -8,    6,
        -6,   1,  -9,   7,   7,  -9,   1,   -6,
        -5,   8,   7,  -6,  -6,   7,   8,   -5,
         6,   1,  -7,  10,  10,  -7,   1,    6,
         4,   5,  20,  -5,  -5,  20,   5,    4,
        18,   0,  19,  13,  13,  19,   0,   18
    };

    static constexpr std::array<int, 64> eg_queen_table = {
       -69, -57, -47, -26, -26, -47, -57,  -69,
       -55, -31, -22,  -4,  -4, -22, -31,  -55,
       -39, -18,  -9,   3,   3,  -9, -18,  -39,
       -23,  -3,  13,  24,  24,  13,  -3,  -23,
       -29,  -6,   9,  21,  21,   9,  -6,  -29,
       -38, -18, -12,   1,   1, -12, -18,  -38,
       -50, -27, -24,  -8,  -8, -24, -27,  -50,
       -75, -52, -43, -36, -36, -43, -52,  -75
    };

    static constexpr std::array<int, 64> eg_king_table = {
         1,  45,  85,  76,  76,  85,  45,    1,
        53, 100, 133, 135, 135, 133, 100,   53,
        88, 130, 169, 175, 175, 169, 130,   88,
       103, 156, 172, 172, 172, 172, 156,  103,
        96, 166, 199, 199, 199, 199, 166,   96,
        92, 172, 184, 191, 191, 184, 172,   92,
        47, 121, 116, 131, 131, 116, 121,   47,
        11,  59,  73,  78,  78,  73,  59,   11
    };

    static constexpr std::array<std::array<int, 64>, 6> mg_tables = {
        mg_pawn_table, mg_knight_table, mg_bishop_table,
        mg_rook_table, mg_queen_table, mg_king_table
    };
    static constexpr std::array<std::array<int, 64>, 6> eg_tables = {
        eg_pawn_table, eg_knight_table, eg_bishop_table,
        eg_rook_table, eg_queen_table, eg_king_table
    };

    // Precompute mg and eg tables at compilation
    static constexpr auto mg_table = []() {
        std::array<std::array<int, 64>, 6> result{};
        for (size_t pt = 0; pt < 6; ++pt) {
            for (size_t sq = 0; sq < 64; ++sq) {
                result[pt][sq] = mg_value[pt] + mg_tables[pt][sq];
            }
        }
        return result;
    }();

    static constexpr auto eg_table = []() {
        std::array<std::array<int, 64>, 6> result{};
        for (size_t pt = 0; pt < 6; ++pt) {
            for (size_t sq = 0; sq < 64; ++sq) {
                result[pt][sq] = eg_value[pt] + eg_tables[pt][sq];
            }
        }
        return result;
    }();   
    
}  // namespace pst

namespace eval {
    // Mobility bonus arrays (directly from Stockfish)
    constexpr std::array<int, 9> knight_mobility_mg = {-62, -53, -12, -4, 3, 13, 22, 28, 33};
    constexpr std::array<int, 9> knight_mobility_eg = {-81, -56, -31, -16, 5, 11, 17, 20, 25};
    
    constexpr std::array<int, 14> bishop_mobility_mg = {-48, -20, 16, 26, 38, 51, 55, 63, 63, 68, 81, 81, 91, 98};
    constexpr std::array<int, 14> bishop_mobility_eg = {-59, -23, -3, 13, 24, 42, 54, 57, 65, 73, 78, 86, 88, 97};
    
    constexpr std::array<int, 15> rook_mobility_mg = {-60, -20, 2, 3, 3, 11, 22, 31, 40, 40, 41, 48, 57, 57, 62};
    constexpr std::array<int, 15> rook_mobility_eg = {-78, -17, 23, 39, 70, 99, 103, 121, 134, 139, 158, 164, 168, 169, 172};
    
    constexpr std::array<int, 28> queen_mobility_mg = {-30, -12, -8, -9, 20, 23, 23, 35, 38, 53, 64, 65, 65, 66, 67, 67, 72, 72, 77, 79, 93, 108, 108, 108, 110, 114, 114, 116};
    constexpr std::array<int, 28> queen_mobility_eg = {-48, -30, -7, 19, 40, 55, 59, 75, 78, 96, 96, 100, 121, 127, 131, 133, 136, 141, 147, 150, 151, 168, 168, 171, 182, 182, 192, 219};
}  // namespace eval
