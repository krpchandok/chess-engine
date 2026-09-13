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
    //
    // `weights` is optional (nullptr means "use the classical evaluate()").
    // Passing it through lets search_nnue() reuse this exact same search
    // tree/pruning logic and only swap out the leaf scoring function.
    int alpha_beta(Board& board, int depth, int alpha, int beta, bool is_white,
                    const NNUEWeights* weights) {
        if (depth == 0) {
            int white_score = (weights != nullptr && weights->loaded)
                ? evaluate_nnue(board, *weights)
                : evaluate(board);
            return is_white ? white_score : -white_score;
        }

        auto moves = generate_legal_moves(board, is_white);
        if (moves.empty()) {
            return is_in_check(board, is_white) ? -100000 : 0;
        }

        int best = std::numeric_limits<int>::min() + 1;
        for (const auto& move : moves) {
            UndoInfo undo = make_move(board, move);
            int score = -alpha_beta(board, depth - 1, -beta, -alpha, !is_white, weights);
            unmake_move(board, move, undo);

            best = std::max(best, score);
            alpha = std::max(alpha, score);
            if (alpha >= beta) break;
        }
        return best;
    }

    SearchResult search_impl(const Board& board_in, int depth, const NNUEWeights* weights) {
        SearchResult result;
        bool is_white = board_in.is_white_to_move();
        Board board = board_in;

        auto moves = generate_legal_moves(board, is_white);
        if (moves.empty()) return result;

        int alpha = std::numeric_limits<int>::min() + 1;
        int beta = std::numeric_limits<int>::max() - 1;

        for (const auto& move : moves) {
            UndoInfo undo = make_move(board, move);
            int score = -alpha_beta(board, depth - 1, -beta, -alpha, !is_white, weights);
            unmake_move(board, move, undo);

            if (!result.has_move || score > result.score) {
                result.score = score;
                result.best_move = move;
                result.has_move = true;
            }
            alpha = std::max(alpha, score);
        }

        return result;
    }
}

SearchResult search(const Board& board, int depth) {
    return search_impl(board, depth, nullptr);
}

SearchResult search_nnue(const Board& board, int depth, const NNUEWeights& weights) {
    return search_impl(board, depth, &weights);
}