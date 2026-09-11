// Loads trained NNUE weights and runs a quick A/B: classical alpha-beta
// eval vs. learned eval, on the same search tree, from the same position.
//
// Usage: nnue_demo <weights_path> [depth]

#include "board.hpp"
#include "piece.hpp"
#include "search.hpp"
#include "nnue_eval.hpp"
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

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <weights_path> [depth]\n";
        return 1;
    }
    std::string weights_path = argv[1];
    int depth = argc >= 3 ? std::stoi(argv[2]) : 4;

    init_attack_tables();

    NNUEWeights weights;
    if (!load_nnue_weights(weights_path, weights)) {
        std::cerr << "Failed to load weights from " << weights_path << "\n";
        return 1;
    }

    Board board;

    auto t0 = std::chrono::steady_clock::now();
    SearchResult classical = search(board, depth);
    auto t1 = std::chrono::steady_clock::now();
    SearchResult learned = search_nnue(board, depth, weights);
    auto t2 = std::chrono::steady_clock::now();

    auto classical_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto learned_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    std::cout << "classical eval  depth=" << depth
              << "  move=" << square_name(classical.best_move.from) << square_name(classical.best_move.to)
              << "  score=" << classical.score
              << "  (" << classical_ms << "ms)\n";

    std::cout << "learned eval    depth=" << depth
              << "  move=" << square_name(learned.best_move.from) << square_name(learned.best_move.to)
              << "  score=" << learned.score
              << "  (" << learned_ms << "ms)\n";

    return 0;
}