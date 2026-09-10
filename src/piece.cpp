#include "piece.hpp"
#include <bit>

const int DIAG_DELTAS[4][2]  = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
const int ORTHO_DELTAS[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

std::array<uint64_t, 64> knight_attacks;
std::array<uint64_t, 64> king_attacks;

uint64_t ray_attacks(int square, const int deltas[4][2], uint64_t all_occupancy, uint64_t own_occupancy) {
    uint64_t attacks = 0;
    int file = square % 8;
    int rank = square / 8;

    for (int d = 0; d < 4; ++d) {
        int f = file, r = rank;
        while (true) {
            f += deltas[d][0];
            r += deltas[d][1];
            if (f < 0 || f > 7 || r < 0 || r > 7) break;

            int target = r * 8 + f;
            uint64_t bit = 1ULL << target;
            attacks |= bit;

            if (all_occupancy & bit) break;
        }
    }

    return attacks & ~own_occupancy;
}

void init_attack_tables() {
    constexpr int knight_deltas[8][2] = {
        { 1,  2}, { 2,  1}, { 2, -1}, { 1, -2},
        {-1, -2}, {-2, -1}, {-2,  1}, {-1,  2}
    };
    constexpr int king_deltas[8][2] = {
        { 1,  0}, { 1,  1}, { 0,  1}, {-1,  1},
        {-1,  0}, {-1, -1}, { 0, -1}, { 1, -1}
    };

    for (int square = 0; square < 64; ++square) {
        int file = square % 8;
        int rank = square / 8;

        uint64_t n_attacks = 0;
        for (auto& d : knight_deltas) {
            int f = file + d[0], r = rank + d[1];
            if (f < 0 || f > 7 || r < 0 || r > 7) continue;
            n_attacks |= (1ULL << (r * 8 + f));
        }
        knight_attacks[square] = n_attacks;

        uint64_t k_attacks = 0;
        for (auto& d : king_deltas) {
            int f = file + d[0], r = rank + d[1];
            if (f < 0 || f > 7 || r < 0 || r > 7) continue;
            k_attacks |= (1ULL << (r * 8 + f));
        }
        king_attacks[square] = k_attacks;
    }
}

uint64_t Knight::attacks_from(const Board& board, int square, bool is_white) const {
    uint64_t own_occ = is_white ? board.get_white_occupancy() : board.get_black_occupancy();
    return knight_attacks[square] & ~own_occ;
}

uint64_t King::attacks_from(const Board& board, int square, bool is_white) const {
    uint64_t own_occ = is_white ? board.get_white_occupancy() : board.get_black_occupancy();
    return king_attacks[square] & ~own_occ;
}

uint64_t Bishop::attacks_from(const Board& board, int square, bool is_white) const {
    uint64_t own_occ = is_white ? board.get_white_occupancy() : board.get_black_occupancy();
    return ray_attacks(square, DIAG_DELTAS, board.get_all_occupancy(), own_occ);
}

uint64_t Rook::attacks_from(const Board& board, int square, bool is_white) const {
    uint64_t own_occ = is_white ? board.get_white_occupancy() : board.get_black_occupancy();
    return ray_attacks(square, ORTHO_DELTAS, board.get_all_occupancy(), own_occ);
}

uint64_t Queen::attacks_from(const Board& board, int square, bool is_white) const {
    uint64_t own_occ = is_white ? board.get_white_occupancy() : board.get_black_occupancy();
    uint64_t occ = board.get_all_occupancy();
    return ray_attacks(square, DIAG_DELTAS, occ, own_occ) | ray_attacks(square, ORTHO_DELTAS, occ, own_occ);
}

uint64_t Pawn::attacks_from(const Board& board, int square, bool is_white) const {
    (void)board;
    int file = square % 8;
    int rank = square / 8;
    int dir = is_white ? 1 : -1;

    uint64_t attacks = 0;
    for (int df : {-1, 1}) {
        int f = file + df, r = rank + dir;
        if (f < 0 || f > 7 || r < 0 || r > 7) continue;
        attacks |= (1ULL << (r * 8 + f));
    }
    return attacks;
}

uint64_t Pawn::pushes_from(const Board& board, int square, bool is_white) const {
    int file = square % 8;
    int rank = square / 8;
    uint64_t all_occ = board.get_all_occupancy();
    int dir = is_white ? 1 : -1;
    int start_rank = is_white ? 1 : 6;

    uint64_t pushes = 0;
    int one_rank = rank + dir;
    if (one_rank < 0 || one_rank > 7) return 0;

    int one_sq = one_rank * 8 + file;
    if (!(all_occ & (1ULL << one_sq))) {
        pushes |= (1ULL << one_sq);

        if (rank == start_rank) {
            int two_rank = rank + 2 * dir;
            int two_sq = two_rank * 8 + file;
            if (!(all_occ & (1ULL << two_sq))) {
                pushes |= (1ULL << two_sq);
            }
        }
    }
    return pushes;
}