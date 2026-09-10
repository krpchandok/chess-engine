#pragma once
#include <vector>
#include "board.hpp"
#include "move.hpp"

std::vector<Move> generate_pseudo_legal_moves(const Board& board, bool is_white);
std::vector<Move> generate_legal_moves(const Board& board, bool is_white);

bool is_square_attacked(const Board& board, int square, bool by_white);
bool is_in_check(const Board& board, bool is_white);

Board make_move(const Board& board, const Move& move);