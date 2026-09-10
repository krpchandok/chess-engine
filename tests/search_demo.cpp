#include "board.hpp"
#include "search.hpp"
#include "piece.hpp"
#include <iostream>
#include <chrono>

namespace {
    std::string square_name(int sq) {
        std::string s;
        s += static_cast<char>('a' + sq % 8);
        s += static_cast<char>('1' + sq / 8);
        return s;
    }
}

int main() {
    init_attack_tables();
    Board board;

    for (int depth = 1; depth <= 4; ++depth) {
        auto start = std::chrono::steady_clock::now();
        SearchResult result = search(board, depth);
        auto elapsed = std::chrono::steady_clock::now() - start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        if (!result.has_move) {
            std::cout << "depth " << depth << ": no legal moves (unexpected at start position)\n";
            return 1;
        }

        std::cout << "depth " << depth << ": "
                  << square_name(result.best_move.from) << square_name(result.best_move.to)
                  << "  score=" << result.score
                  << "  (" << ms << "ms)\n";
    }

    return 0;
}