#include "board.hpp"
#include "movegen.hpp"
#include "piece.hpp"
#include <iostream>

long long perft(const Board& board, int depth, bool is_white) {
    if (depth == 0) return 1;

    long long nodes = 0;
    for (const auto& move : generate_legal_moves(board, is_white)) {
        Board next = make_move(board, move);
        nodes += perft(next, depth - 1, !is_white);
    }
    return nodes;
}

int main() {
    init_attack_tables();
    Board board;

    long long expected[] = {20, 400, 8902, 197281};

    bool all_passed = true;
    for (int depth = 1; depth <= 4; ++depth) {
        long long result = perft(board, depth, true);
        bool passed = (result == expected[depth - 1]);
        all_passed &= passed;

        std::cout << "depth " << depth << ": got " << result
                  << ", expected " << expected[depth - 1]
                  << (passed ? "  [PASS]" : "  [FAIL]") << "\n";

        if (!passed) break;
    }

    return all_passed ? 0 : 1;
}