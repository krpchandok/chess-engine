#include "zobrist.hpp"
#include <random>
#include <bit>

namespace {
    uint64_t piece_keys[PIECE_CODE_NB][64];
    uint64_t side_to_move_key;
    uint64_t castling_keys[16];
    uint64_t en_passant_keys[8];
}

void init_zobrist_keys() {
    std::mt19937_64 rng(0xC0FFEE123456789ULL);

    for (int pc = 0; pc < PIECE_CODE_NB; ++pc)
        for (int sq = 0; sq < 64; ++sq)
            piece_keys[pc][sq] = rng();

    side_to_move_key = rng();
    for (auto& k : castling_keys) k = rng();
    for (auto& k : en_passant_keys) k = rng();
}

uint64_t compute_zobrist_hash(const Board& board) {
    uint64_t hash = 0;

    for (int pc = 0; pc < PIECE_CODE_NB; ++pc) {
        uint64_t bb = board.get_bitboard(static_cast<PieceCode>(pc));
        while (bb) {
            int sq = 0;
            while ((bb & 1ULL) == 0) {
                bb >>= 1;
                ++sq;
            }
            bb &= bb - 1;
            hash ^= piece_keys[pc][sq];
        }
    }

    if (board.is_white_to_move()) hash ^= side_to_move_key;
    hash ^= castling_keys[board.get_castling_rights()];

    if (board.get_en_passant_square() != -1) {
        hash ^= en_passant_keys[board.get_en_passant_square() % 8];
    }

    return hash;
}