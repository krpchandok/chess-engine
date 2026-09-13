#include "parallel.hpp"
#include "movegen.hpp"
#include "eval.hpp"
#include "zobrist.hpp"
#include <limits>
#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

namespace {
    int alpha_beta(Board& board, int depth, int alpha, int beta, bool is_white,
                   TranspositionTable& tt, const std::atomic<bool>& stop,
                   std::atomic<long long>& node_counter) {
        if (stop.load(std::memory_order_relaxed)) return 0;
        node_counter.fetch_add(1, std::memory_order_relaxed);

        uint64_t key = compute_zobrist_hash(board);
        int alpha_orig = alpha;

        TTEntry entry;
        if (tt.probe(key, entry) && entry.depth >= depth) {
            if (entry.flag == TTFlag::EXACT) {
                return entry.score;
            } else if (entry.flag == TTFlag::LOWER) {
                alpha = std::max(alpha, entry.score);
            } else {
                beta = std::min(beta, entry.score);
            }
            if (alpha >= beta) return entry.score;
        }

        if (depth == 0) {
            int white_score = evaluate(board);
            return is_white ? white_score : -white_score;
        }

        auto moves = generate_legal_moves(board, is_white);
        if (moves.empty()) {
            return is_in_check(board, is_white) ? -100000 : 0;
        }

        int best = std::numeric_limits<int>::min() + 1;
        Move best_move = moves[0];

        for (const auto& move : moves) {
            UndoInfo undo = make_move(board, move);
            int score = -alpha_beta(board, depth - 1, -beta, -alpha, !is_white, tt, stop, node_counter);
            unmake_move(board, move, undo);

            if (score > best) {
                best = score;
                best_move = move;
            }
            alpha = std::max(alpha, score);
            if (alpha >= beta) break;
        }

        TTFlag flag = (best <= alpha_orig) ? TTFlag::UPPER
                     : (best >= beta)       ? TTFlag::LOWER
                                            : TTFlag::EXACT;
        tt.store(key, depth, best, flag, best_move);

        return best;
    }

    SearchResult search_root(const Board& board_in, int depth, TranspositionTable& tt,
                              const std::atomic<bool>& stop, std::atomic<long long>& node_counter) {
        SearchResult result;
        bool is_white = board_in.is_white_to_move();
        Board board = board_in;

        auto moves = generate_legal_moves(board, is_white);
        if (moves.empty()) return result;

        int alpha = std::numeric_limits<int>::min() + 1;
        int beta = std::numeric_limits<int>::max() - 1;

        for (const auto& move : moves) {
            if (stop.load(std::memory_order_relaxed)) break;

            UndoInfo undo = make_move(board, move);
            int score = -alpha_beta(board, depth - 1, -beta, -alpha, !is_white, tt, stop, node_counter);
            unmake_move(board, move, undo);

            if (!result.has_move || score > result.score) {
                result.score = score;
                result.best_move = move;
                result.has_move = true;
            }
            alpha = std::max(alpha, score);
        }

        result.depth_reached = depth;
        return result;
    }
}

SearchResult search_parallel(const Board& board, const SearchLimits& limits,
                              int num_threads, TranspositionTable& tt) {
    std::atomic<bool> stop{false};
    std::atomic<long long> node_counter{0};

    auto deadline = std::chrono::steady_clock::now() + limits.time_limit;

    std::thread timer([&]() {
        std::this_thread::sleep_until(deadline);
        stop.store(true, std::memory_order_relaxed);
    });

    std::vector<std::thread> helpers;
    for (int t = 1; t < num_threads; ++t) {
        helpers.emplace_back([&]() {
            for (int depth = 1; depth <= limits.max_depth; ++depth) {
                if (stop.load(std::memory_order_relaxed)) break;
                search_root(board, depth, tt, stop, node_counter);
            }
        });
    }

    // main_result only ever gets set from a depth that finished *before*
    // stop fired — a depth interrupted mid-way can return a corrupted
    // comparison (alpha_beta returns a fabricated 0 the instant stop is
    // seen, which can look artificially good/bad and get wrongly picked
    // as best), so an interrupted depth's result is never trusted.
    //
    // fallback_result exists only for the edge case where even depth 1
    // gets interrupted (extremely tight time budget) and main_result never
    // gets set at all — in that case, returning *some* legal move (even
    // one chosen via a possibly-corrupted comparison) is still better than
    // reporting no legal moves in a position that has them.
    SearchResult main_result;
    SearchResult fallback_result;

    for (int depth = 1; depth <= limits.max_depth; ++depth) {
        if (stop.load(std::memory_order_relaxed)) break;

        SearchResult r = search_root(board, depth, tt, stop, node_counter);
        if (r.has_move) {
            fallback_result = r;
            if (!stop.load(std::memory_order_relaxed)) {
                main_result = r;
            }
        }
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& h : helpers) h.join();
    timer.join();

    SearchResult final_result = main_result.has_move ? main_result : fallback_result;
    final_result.nodes_searched = node_counter.load(std::memory_order_relaxed);
    return final_result;
}