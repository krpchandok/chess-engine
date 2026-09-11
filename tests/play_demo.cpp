#include "board.hpp"
#include "movegen.hpp"
#include "parallel.hpp"
#include "tt.hpp"
#include "zobrist.hpp"
#include "piece.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <chrono>

namespace {
    std::string square_name(int sq) {
        std::string s;
        s += static_cast<char>('a' + sq % 8);
        s += static_cast<char>('1' + sq / 8);
        return s;
    }

    int parse_square(const std::string& s) {
        int file = s[0] - 'a';
        int rank = s[1] - '1';
        return rank * 8 + file;
    }

    char piece_char(int8_t pc) {
        switch (pc) {
            case WP: return 'P'; case WN: return 'N'; case WB: return 'B';
            case WR: return 'R'; case WQ: return 'Q'; case WK: return 'K';
            case BP: return 'p'; case BN: return 'n'; case BB: return 'b';
            case BR: return 'r'; case BQ: return 'q'; case BK: return 'k';
            default: return '.';
        }
    }

    void print_board(const Board& board) {
        std::cout << "\n";
        for (int rank = 7; rank >= 0; --rank) {
            std::cout << (rank + 1) << "  ";
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                std::cout << piece_char(board.piece_at(sq)) << " ";
            }
            std::cout << "\n";
        }
        std::cout << "   a b c d e f g h\n\n";
    }

    std::optional<Move> find_matching_move(const Board& board, bool is_white, const std::string& uci_str) {
        if (uci_str.size() < 4) return std::nullopt;

        int from = parse_square(uci_str.substr(0, 2));
        int to = parse_square(uci_str.substr(2, 2));

        PromoPiece promo = PromoPiece::NONE;
        if (uci_str.size() == 5) {
            switch (uci_str[4]) {
                case 'q': promo = PromoPiece::QUEEN; break;
                case 'r': promo = PromoPiece::ROOK; break;
                case 'b': promo = PromoPiece::BISHOP; break;
                case 'n': promo = PromoPiece::KNIGHT; break;
            }
        }

        for (const auto& m : generate_legal_moves(board, is_white)) {
            if (m.from == from && m.to == to && m.promotion == promo) {
                return m;
            }
        }
        return std::nullopt;
    }
}

int main() {
    init_attack_tables();
    init_zobrist_keys();

    Board board;
    TranspositionTable tt(1 << 20);

    std::cout << "You are White. Enter moves like e2e4 (or e7e8q to promote). Type 'quit' to exit.\n";

    while (true) {
        print_board(board);
        std::cout << "DEBUG castling_rights: " << (int)board.get_castling_rights() << "\n";
        bool is_white = board.is_white_to_move();
        auto legal = generate_legal_moves(board, is_white);

        if (legal.empty()) {
            if (is_in_check(board, is_white)) {
                std::cout << (is_white ? "Black" : "White") << " wins by checkmate.\n";
            } else {
                std::cout << "Draw by stalemate.\n";
            }
            break;
        }

        if (is_white) {
            std::cout << "Your move: ";
            std::string input;
            if (!(std::cin >> input) || input == "quit") break;

            auto move = find_matching_move(board, true, input);
            if (!move) {
                std::cout << "Illegal move, try again.\n";
                continue;
            }
            board = make_move(board, *move);
        } else {
            std::cout << "Engine is thinking...\n";
            SearchLimits limits;
            limits.time_limit = std::chrono::milliseconds(2000);
            auto result = search_parallel(board, limits, 4, tt);

            std::cout << "Engine plays: "
                      << square_name(result.best_move.from) << square_name(result.best_move.to)
                      << "  (depth " << result.depth_reached
                      << ", " << result.nodes_searched << " nodes)\n";

            board = make_move(board, result.best_move);
        }
    }

    return 0;
}