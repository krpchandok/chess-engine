#pragma once
#include <cstdint>
#include "board.hpp"

extern const int DIAG_DELTAS[4][2];
extern const int ORTHO_DELTAS[4][2];
uint64_t ray_attacks(int square, const int deltas[4][2], uint64_t all_occupancy, uint64_t own_occupancy);

extern std::array<uint64_t, 64> knight_attacks;
extern std::array<uint64_t, 64> king_attacks;
void init_attack_tables();

class Piece {
public:
    virtual ~Piece() = default;
    virtual uint64_t attacks_from(const Board& board, int square, bool is_white) const = 0;
};

class Knight : public Piece {
public:
    uint64_t attacks_from(const Board& board, int square, bool is_white) const override;
};

class King : public Piece {
public:
    uint64_t attacks_from(const Board& board, int square, bool is_white) const override;
};

class Bishop : public Piece {
public:
    uint64_t attacks_from(const Board& board, int square, bool is_white) const override;
};

class Rook : public Piece {
public:
    uint64_t attacks_from(const Board& board, int square, bool is_white) const override;
};

class Queen : public Piece {
public:
    uint64_t attacks_from(const Board& board, int square, bool is_white) const override;
};

class Pawn : public Piece {
public:
    uint64_t attacks_from(const Board& board, int square, bool is_white) const override;
    uint64_t pushes_from(const Board& board, int square, bool is_white) const;
};