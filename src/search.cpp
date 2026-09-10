#include "search.hpp"
#include "movegen.hpp"
#include "eval.hpp"
#include <limits>
#include <algorithm>

namespace {
    // Negamax form: always returns a score from the perspective of the side
    // to move at `board`. Each recursive call negates the child's score and
    // swaps alpha/beta — this is what lets one function handle both White's
    // and Black's turns without separate max/min branches.
    int alpha_beta(const Board& board, int depth, int alpha, int beta, bool is_white) {
        if (depth == 0) {
            int white_score = evaluate(board);
            return is_white ? white_score : -white_score;
        }

        auto moves = generate_legal_moves(board, is_white);
        if (moves.empty()) {
            return is_in_check(board, is_white) ? -100000 : 0;
        }

        int best = std::numeric_limits<int>::min() + 1;
        for (const auto& move : moves) {
            Board next = make_move(board, move);
            int score = -alpha_beta(next, depth - 1, -beta, -alpha, !is_white);

            best = std::max(best, score);
            alpha = std::max(alpha, score);
            if (alpha >= beta) break;
        }
        return best;
    }
}

SearchResult search(const Board& board, int depth) {
    SearchResult result;
    bool is_white = board.is_white_to_move();

    auto moves = generate_legal_moves(board, is_white);
    if (moves.empty()) return result;

    int alpha = std::numeric_limits<int>::min() + 1;
    int beta = std::numeric_limits<int>::max() - 1;

    for (const auto& move : moves) {
        Board next = make_move(board, move);
        int score = -alpha_beta(next, depth - 1, -beta, -alpha, !is_white);

        if (!result.has_move || score > result.score) {
            result.score = score;
            result.best_move = move;
            result.has_move = true;
        }
        alpha = std::max(alpha, score);
    }

    return result;
}