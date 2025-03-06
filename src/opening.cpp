#include "opening.h"
#include <fstream>
#include <random>

void OpeningBook::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        return;

    struct BinEntry {
        uint64_t key;
        uint16_t move;
        uint16_t weight;
        uint32_t learn;
    };

    BinEntry entry;

    //     while(file.read(reinterpret_cast<char*>(&entry), sizeof(entry))) {
    //         // Convert binary move to chess-library Move
    //         Square from = Square(entry.move & 0x3F);
    //         Square to = Square((entry.move >> 6) & 0x3F);
    //         Move::PromotionType promo = Move::PromotionType((entry.move >> 12) & 0x7);

    //         Move move = Move(from, to, promo);
    //         book[entry.key].push_back({move, entry.weight, entry.learn});
    //     }
    // }
}


bool OpeningBook::probe(const Board& board, Move& result) const {
    auto it = book.find(board.hash());
    if (it == book.end())
        return false;

    const auto& entries = it->second;
    if (entries.empty())
        return false;

    // Select weighted random move
    int totalWeight = 0;
    for (const auto& entry : entries)
        totalWeight += entry.weight;

    static std::mt19937             gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, totalWeight - 1);
    int                             pick = dist(gen);

    for (const auto& entry : entries)
    {
        pick -= entry.weight;
        if (pick < 0)
        {
            result = entry.move;
            return true;
        }
    }
    return false;
}