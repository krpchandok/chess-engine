#include "movegen.hpp"
#include "piece.hpp"
#include <cstdlib>

namespace {
    Knight knight_logic;
    Bishop bishop_logic;
    Rook   rook_logic;
    Queen  queen_logic;
    King   king_logic;
    Pawn   pawn_logic;

    int first_set_bit(uint64_t value) {
        int index = 0;
        while ((value & 1ULL) == 0) {
            value >>= 1;
            ++index;
        }
        return index;
    }

    void add_moves_for(std::vector<Move>& moves, const Board& board, bool is_white,
                        uint64_t piece_bb, const Piece& logic) {
        uint64_t enemy_occ = is_white ? board.get_black_occupancy() : board.get_white_occupancy();

        while (piece_bb) {
            int from = first_set_bit(piece_bb);
            piece_bb &= piece_bb - 1;

            uint64_t targets = logic.attacks_from(board, from, is_white);
            while (targets) {
                int to = first_set_bit(targets);
                targets &= targets - 1;

                Move m;
                m.from = static_cast<uint8_t>(from);
                m.to = static_cast<uint8_t>(to);
                m.is_capture = enemy_occ & (1ULL << to);
                moves.push_back(m);
            }
        }
    }

    void add_pawn_moves(std::vector<Move>& moves, const Board& board, bool is_white) {
        uint64_t pawns = is_white ? board.get_white_pawns() : board.get_black_pawns();
        uint64_t enemy_occ = is_white ? board.get_black_occupancy() : board.get_white_occupancy();
        int promo_rank = is_white ? 7 : 0;

        while (pawns) {
            int from = first_set_bit(pawns);
            pawns &= pawns - 1;

            auto emit = [&](int to, bool is_capture, bool is_ep, bool is_double) {
                int to_rank = to / 8;
                if (to_rank == promo_rank) {
                    for (auto promo : {PromoPiece::QUEEN, PromoPiece::ROOK,
                                        PromoPiece::BISHOP, PromoPiece::KNIGHT}) {
                        Move m;
                        m.from = static_cast<uint8_t>(from);
                        m.to = static_cast<uint8_t>(to);
                        m.is_capture = is_capture;
                        m.is_en_passant = is_ep;
                        m.is_double_push = is_double;
                        m.promotion = promo;
                        moves.push_back(m);
                    }
                } else {
                    Move m;
                    m.from = static_cast<uint8_t>(from);
                    m.to = static_cast<uint8_t>(to);
                    m.is_capture = is_capture;
                    m.is_en_passant = is_ep;
                    m.is_double_push = is_double;
                    moves.push_back(m);
                }
            };

            uint64_t pushes = pawn_logic.pushes_from(board, from, is_white);
            while (pushes) {
                int to = first_set_bit(pushes);
                pushes &= pushes - 1;
                bool is_double = std::abs(to / 8 - from / 8) == 2;
                emit(to, false, false, is_double);
            }

            uint64_t attacks = pawn_logic.attacks_from(board, from, is_white);
            while (attacks) {
                int to = first_set_bit(attacks);
                attacks &= attacks - 1;
                uint64_t to_bit = 1ULL << to;

                if (enemy_occ & to_bit) {
                    emit(to, true, false, false);
                } else if (board.get_en_passant_square() == to) {
                    emit(to, true, true, false);
                }
            }
        }
    }
}

std::vector<Move> generate_pseudo_legal_moves(const Board& board, bool is_white) {
    std::vector<Move> moves;

    add_moves_for(moves, board, is_white,
                  is_white ? board.get_white_knights() : board.get_black_knights(), knight_logic);
    add_moves_for(moves, board, is_white,
                  is_white ? board.get_white_bishops() : board.get_black_bishops(), bishop_logic);
    add_moves_for(moves, board, is_white,
                  is_white ? board.get_white_rooks() : board.get_black_rooks(), rook_logic);
    add_moves_for(moves, board, is_white,
                  is_white ? board.get_white_queens() : board.get_black_queens(), queen_logic);
    add_moves_for(moves, board, is_white,
                  is_white ? board.get_white_king() : board.get_black_king(), king_logic);

    add_pawn_moves(moves, board, is_white);

    return moves;
}

bool is_square_attacked(const Board& board, int square, bool by_white) {
    if (knight_attacks[square] & (by_white ? board.get_white_knights() : board.get_black_knights()))
        return true;

    if (king_attacks[square] & (by_white ? board.get_white_king() : board.get_black_king()))
        return true;

    uint64_t occ = board.get_all_occupancy();
    uint64_t rook_rays = ray_attacks(square, ORTHO_DELTAS, occ, 0);
    uint64_t bishop_rays = ray_attacks(square, DIAG_DELTAS, occ, 0);

    uint64_t enemy_rooks_queens = by_white
        ? (board.get_white_rooks() | board.get_white_queens())
        : (board.get_black_rooks() | board.get_black_queens());
    uint64_t enemy_bishops_queens = by_white
        ? (board.get_white_bishops() | board.get_white_queens())
        : (board.get_black_bishops() | board.get_black_queens());

    if (rook_rays & enemy_rooks_queens) return true;
    if (bishop_rays & enemy_bishops_queens) return true;

    int file = square % 8, rank = square / 8;
    int dir = by_white ? -1 : 1;
    uint64_t enemy_pawns = by_white ? board.get_white_pawns() : board.get_black_pawns();

    for (int df : {-1, 1}) {
        int f = file + df, r = rank + dir;
        if (f < 0 || f > 7 || r < 0 || r > 7) continue;
        if (enemy_pawns & (1ULL << (r * 8 + f))) return true;
    }

    return false;
}

bool is_in_check(const Board& board, bool is_white) {
    uint64_t king_bb = is_white ? board.get_white_king() : board.get_black_king();
    if (king_bb == 0) return false;
    int king_sq = first_set_bit(king_bb);
    return is_square_attacked(board, king_sq, !is_white);
}

std::vector<Move> generate_legal_moves(const Board& board, bool is_white) {
    std::vector<Move> legal;
    for (const auto& m : generate_pseudo_legal_moves(board, is_white)) {
        Board next = make_move(board, m);
        if (!is_in_check(next, is_white)) {
            legal.push_back(m);
        }
    }
    return legal;
}

Board make_move(const Board& board, const Move& move) {
    Board next = board;
    bool is_white = board.is_white_to_move();

    int8_t moving_piece = board.piece_at(move.from);
    uint64_t from_bit = 1ULL << move.from;
    uint64_t to_bit = 1ULL << move.to;

    if (move.is_capture && !move.is_en_passant) {
        int8_t captured = board.piece_at(move.to);
        if (captured != EMPTY) {
            auto pc = static_cast<PieceCode>(captured);
            next.set_bitboard(pc, next.get_bitboard(pc) & ~to_bit);
        }
    }

    if (move.is_en_passant) {
        int captured_sq = is_white ? move.to - 8 : move.to + 8;
        PieceCode captured_pc = is_white ? BP : WP;
        next.set_bitboard(captured_pc, next.get_bitboard(captured_pc) & ~(1ULL << captured_sq));
    }

    auto moving_pc = static_cast<PieceCode>(moving_piece);
    uint64_t moving_bb = next.get_bitboard(moving_pc) & ~from_bit;
    next.set_bitboard(moving_pc, moving_bb);

    if (move.promotion != PromoPiece::NONE) {
        PieceCode promo_pc;
        switch (move.promotion) {
            case PromoPiece::QUEEN:  promo_pc = is_white ? WQ : BQ; break;
            case PromoPiece::ROOK:   promo_pc = is_white ? WR : BR; break;
            case PromoPiece::BISHOP: promo_pc = is_white ? WB : BB; break;
            case PromoPiece::KNIGHT: promo_pc = is_white ? WN : BN; break;
            default:                 promo_pc = moving_pc; break;
        }
        next.set_bitboard(promo_pc, next.get_bitboard(promo_pc) | to_bit);
    } else {
        next.set_bitboard(moving_pc, next.get_bitboard(moving_pc) | to_bit);
    }

    next.set_en_passant_square(move.is_double_push ? (is_white ? move.to - 8 : move.to + 8) : -1);

    next.set_white_to_move(!is_white);
    next.update_occupancy();
    next.update_mailbox();

    return next;
}