#pragma once
#include "board.hpp"
#include "move.hpp"

struct SearchResult {
    Move best_move{};
    int score = 0;
    bool has_move = false;  
};

SearchResult search(const Board& board, int depth);