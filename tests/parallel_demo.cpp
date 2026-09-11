#include "board.hpp"
#include "parallel.hpp"
#include "tt.hpp"
#include "zobrist.hpp"
#include "piece.hpp"
#include <iostream>
#include <thread>

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
    init_zobrist_keys();

    Board board;
    SearchLimits limits;
    limits.time_limit = std::chrono::milliseconds(1500);

    unsigned hw = std::thread::hardware_concurrency();
    std::cout << "hardware_concurrency reports: " << hw << " threads\n\n";

    for (int threads : {1, 2, 4}) {
        TranspositionTable tt(1 << 20);
        auto result = search_parallel(board, limits, threads, tt);

        std::cout << threads << " thread(s): "
                  << square_name(result.best_move.from) << square_name(result.best_move.to)
                  << "  depth=" << result.depth_reached
                  << "  score=" << result.score
                  << "  nodes=" << result.nodes_searched
                  << "\n";
    }

    return 0;
}