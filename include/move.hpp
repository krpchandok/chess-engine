#pragma once
#include <cstdint>

enum class PromoPiece : uint8_t { NONE, KNIGHT, BISHOP, ROOK, QUEEN };

struct Move {
    uint8_t from = 0;
    uint8_t to = 0;
    bool is_capture = false;
    bool is_en_passant = false;
    bool is_double_push = false;
    PromoPiece promotion = PromoPiece::NONE;
};