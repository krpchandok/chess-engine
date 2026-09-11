#include "eval.hpp"
#include "piece_tables.hpp"
#include <array>

namespace {
    constexpr int PIECE_VALUES[6] = {100, 320, 330, 500, 900, 0};

    const std::array<int, 64>* TABLES[6] = {
        &PAWN_TABLE, &KNIGHT_TABLE, &BISHOP_TABLE, &ROOK_TABLE, &QUEEN_TABLE, &KING_TABLE
    };

    int mirror_square(int square) {
        int file = square % 8;
        int rank = square / 8;
        return (7 - rank) * 8 + file;
    }

    int positional_score(uint64_t bitboard, const std::array<int, 64>& table, bool is_white) {
        int score = 0;
        while (bitboard) {
            int square = 0;
            while (((bitboard >> square) & 1ULL) == 0) {
                ++square;
            }
            bitboard &= bitboard - 1;
            score += table[is_white ? square : mirror_square(square)];
        }
        return score;
    }

    int popcount(uint64_t bitboard) {
        int count = 0;
        while (bitboard != 0) {
            bitboard &= bitboard - 1;
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

        uint64_t white_bb = board.get_bitboard(white_pc);
        uint64_t black_bb = board.get_bitboard(black_pc);

        int white_count = popcount(white_bb);
        int black_count = popcount(black_bb);
        score += PIECE_VALUES[i] * (white_count - black_count);

        score += positional_score(white_bb, *TABLES[i], true);
        score -= positional_score(black_bb, *TABLES[i], false);
    }

    return score;
}