
#include "board.hpp"
#include "piece.hpp"
#include "movegen.hpp"
#include "search.hpp"
#include "nnue_eval.hpp"
#include <fstream>
#include <iostream>
#include <random>
#include <string>

namespace {
    // Small amount of randomness in move selection during data generation
    // so self-play games don't all collapse into the same lines.
    std::mt19937 rng(std::random_device{}());

    Move pick_move(const Board& board, bool is_white, int depth, double random_move_prob) {
        auto moves = generate_legal_moves(board, is_white);
        std::uniform_real_distribution<double> coin(0.0, 1.0);
        if (!moves.empty() && coin(rng) < random_move_prob) {
            std::uniform_int_distribution<size_t> pick(0, moves.size() - 1);
            return moves[pick(rng)];
        }
        SearchResult result = search(board, depth);
        return result.best_move;
    }
}

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <num_games> <play_depth> <label_depth> <output_path>\n";
        return 1;
    }

    int num_games = std::stoi(argv[1]);
    int play_depth = std::stoi(argv[2]);
    int label_depth = std::stoi(argv[3]);
    std::string output_path = argv[4];

    init_attack_tables();

    std::ofstream out(output_path);
    if (!out) {
        std::cerr << "Could not open output file: " << output_path << "\n";
        return 1;
    }

    long long positions_written = 0;

    for (int game = 0; game < num_games; ++game) {
        Board board;
        bool is_white = true;
        constexpr int MAX_PLIES = 120;

        for (int ply = 0; ply < MAX_PLIES; ++ply) {
            auto legal = generate_legal_moves(board, is_white);
            if (legal.empty()) break; // checkmate or stalemate

            // Skip logging the first few plies (openings are near-symmetric
            // and add little training signal), then log the rest.
            if (ply >= 4) {
                std::array<float, NNUE_INPUT_SIZE> features;
                extract_features(board, features);

                SearchResult label_result = search(board, label_depth);
                int white_relative_label = is_white ? label_result.score : -label_result.score;

                for (float f : features) out << f << ' ';
                out << white_relative_label << '\n';
                ++positions_written;
            }

            // 10% random moves during play keeps games from repeating
            // the same lines every self-play game.
            Move move = pick_move(board, is_white, play_depth, 0.1);
            board = make_move(board, move);
            is_white = !is_white;
        }

        if ((game + 1) % 10 == 0) {
            std::cerr << "game " << (game + 1) << "/" << num_games
                      << "  positions so far: " << positions_written << "\n";
        }
    }

    std::cerr << "Done. Wrote " << positions_written << " positions to " << output_path << "\n";
    return 0;
}