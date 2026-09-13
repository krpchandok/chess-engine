#include "board.hpp"
#include "movegen.hpp"
#include "piece.hpp"
#include <iostream>
#include <string>
#include <vector>

long long perft(Board& board, int depth, bool is_white) {
    if (depth == 0) return 1;

    long long nodes = 0;
    for (const auto& move : generate_legal_moves(board, is_white)) {
        UndoInfo undo = make_move(board, move);
        nodes += perft(board, depth - 1, !is_white);
        unmake_move(board, move, undo);
    }
    return nodes;
}

struct Case {
    std::string name;
    std::string fen;
    std::vector<long long> expected;
};

int main() {
    init_attack_tables();
    bool all_passed = true;

    std::vector<Case> cases = {
        {"startpos", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         {20, 400, 8902, 197281, 4865609}},
        {"kiwipete (castling)", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
         {48, 2039, 97862, 4085603}},
        {"position 3 (en passant)", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
         {14, 191, 2812, 43238, 674624}},
        {"position 4 (promotion)", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
         {6, 264, 9467, 422333}},
    };

    for (auto& c : cases) {
        Board board;
        board.set_from_fen(c.fen);
        bool is_white = board.is_white_to_move();

        for (size_t d = 0; d < c.expected.size(); ++d) {
            long long got = perft(board, static_cast<int>(d) + 1, is_white);
            bool passed = (got == c.expected[d]);
            all_passed &= passed;

            std::cout << c.name << " depth " << d + 1 << ": got " << got
                      << ", expected " << c.expected[d]
                      << (passed ? "  [PASS]" : "  [FAIL]") << "\n";

            if (!passed) break;
        }
    }

    return all_passed ? 0 : 1;
}
