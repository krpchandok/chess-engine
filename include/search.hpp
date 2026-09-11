#pragma once
#include "board.hpp"
#include "move.hpp"
#include "nnue_eval.hpp"

struct SearchResult {
    Move best_move{};
    int score = 0;
    bool has_move = false;
    int depth_reached = 0;
    long long nodes_searched = 0;
};

SearchResult search(const Board& board, int depth);
SearchResult search_nnue(const Board& board, int depth, const NNUEWeights& weights);