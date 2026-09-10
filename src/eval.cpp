#include "eval.hpp"
#include <cstdint>

namespace {
    constexpr int PIECE_VALUES[6] = {100, 320, 330, 500, 900, 0};

    int count_bits(std::uint64_t value) {
        int count = 0;
        while (value != 0) {
            value &= value - 1;
            ++count;
        }
        return count;
    }
}

int evaluate(const Board& board) {
    int score = 0;
    for (int i = 0; i < 6; ++i) {
        auto white_pc = static_cast<PieceCode>(WP + i);
        auto black_pc = static_cast<PieceCode>(BP + i);

        int white_count = count_bits(board.get_bitboard(white_pc));
        int black_count = count_bits(board.get_bitboard(black_pc));

        score += PIECE_VALUES[i] * (white_count - black_count);
    }
    return score;
}