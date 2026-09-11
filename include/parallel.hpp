#pragma once
#include <chrono>
#include "board.hpp"
#include "search.hpp"
#include "tt.hpp"

struct SearchLimits {
    int max_depth = 64;
    std::chrono::milliseconds time_limit{2000};
};

SearchResult search_parallel(const Board& board, const SearchLimits& limits,
                              int num_threads, TranspositionTable& tt);