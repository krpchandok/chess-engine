#pragma once
#include <vector>
#include "board.hpp"
#include "move.hpp"

struct UndoInfo {
    int8_t moving_piece = EMPTY;
    int8_t captured_piece = EMPTY;
    int captured_square = -1;
    int prev_en_passant = -1;
    uint8_t prev_castling_rights = 0;
};

std::vector<Move> generate_pseudo_legal_moves(const Board& board, bool is_white);
std::vector<Move> generate_legal_moves(const Board& board, bool is_white);

bool is_square_attacked(const Board& board, int square, bool by_white);
bool is_in_check(const Board& board, bool is_white);

UndoInfo make_move(Board& board, const Move& move);
void unmake_move(Board& board, const Move& move, const UndoInfo& undo);