#include "board.hpp"
#include "movegen.hpp"
#include "parallel.hpp"
#include "tt.hpp"
#include "zobrist.hpp"
#include "piece.hpp"
#include <iostream>

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

    // Classic back-rank mate: Black's king on g8 is boxed in by its own
    // pawns (f7/g7/h7), with f8 and h8 the only escape squares — both of
    // which a rook landing on e8 covers along the open rank. White to move,
    // Re1-e8 is mate in one.
    Board board;
    board.set_from_fen("6k1/5ppp/8/8/8/8/8/4R2K w - - 0 1 ");

    TranspositionTable tt(1 << 16);
    SearchLimits limits;
    limits.time_limit = std::chrono::milliseconds(500);
    limits.max_depth = 3;

    auto result = search_parallel(board, limits, 2, tt);

    std::cout << "engine found: "
              << square_name(result.best_move.from) << square_name(result.best_move.to)
              << "  score=" << result.score << "\n";

    bool found_correct_move = (result.best_move.from == 4 && result.best_move.to == 60);
    std::cout << "found the mating move (e1e8): " << found_correct_move << "\n";

    Board after = make_move(board, result.best_move);
    bool black_has_moves = !generate_legal_moves(after, false).empty();
    bool black_in_check = is_in_check(after, false);

    std::cout << "black in check after: " << black_in_check << "\n";
    std::cout << "black has legal moves after: " << black_has_moves << "\n";
    std::cout << "checkmate confirmed: " << (black_in_check && !black_has_moves) << "\n";

    return 0;
}