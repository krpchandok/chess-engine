#include "uci.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "search.hpp"
#include "piece.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <optional>

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

    std::string move_to_uci(const Move& m) {
        std::string s = square_name(m.from) + square_name(m.to);
        switch (m.promotion) {
            case PromoPiece::QUEEN:  s += 'q'; break;
            case PromoPiece::ROOK:   s += 'r'; break;
            case PromoPiece::BISHOP: s += 'b'; break;
            case PromoPiece::KNIGHT: s += 'n'; break;
            default: break;
        }
        return s;
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

    void handle_position(Board& board, std::istringstream& iss) {
        std::string token;
        iss >> token;

        if (token == "startpos") {
            board.initialize();
            iss >> token;
        } else if (token == "fen") {
            std::string fen;
            for (int i = 0; i < 6 && iss >> token && token != "moves"; ++i) {
                fen += token + " ";
            }
            board.set_from_fen(fen);
        }

        if (token == "moves") {
            std::string move_str;
            while (iss >> move_str) {
                bool is_white = board.is_white_to_move();
                auto move = find_matching_move(board, is_white, move_str);
                if (!move) {
                    std::cerr << "info string illegal move in position command: " << move_str << "\n";
                    break;
                }
                board = make_move(board, *move);
            }
        }
    }

    void handle_go(Board& board, std::istringstream& iss) {
        int depth = 4;
        std::string token;
        while (iss >> token) {
            if (token == "depth") {
                iss >> depth;
            }
        }

        SearchResult result = search(board, depth);
        if (result.has_move) {
            std::cout << "bestmove " << move_to_uci(result.best_move) << "\n";
        } else {
            std::cout << "bestmove 0000\n";
        }
        std::cout.flush();
    }
}

void run_uci_loop() {
    init_attack_tables();
    Board board;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "uci") {
            std::cout << "id name Kirpa's Chess Engine\n";
            std::cout << "id author Kirpa\n";
            std::cout << "uciok\n";
        } else if (cmd == "isready") {
            std::cout << "readyok\n";
        } else if (cmd == "ucinewgame") {
            board.initialize();
        } else if (cmd == "position") {
            handle_position(board, iss);
        } else if (cmd == "go") {
            handle_go(board, iss);
        } else if (cmd == "quit") {
            break;
        }
        std::cout.flush();
    }
}