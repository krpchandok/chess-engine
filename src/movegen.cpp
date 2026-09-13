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

    void add_castling_moves(std::vector<Move>& moves, const Board& board, bool is_white) {
        uint8_t rights = board.get_castling_rights();
        uint8_t king_side_flag = is_white ? CASTLE_WK : CASTLE_BK;
        uint8_t queen_side_flag = is_white ? CASTLE_WQ : CASTLE_BQ;

        if (!(rights & (king_side_flag | queen_side_flag))) return;

        uint64_t king_bb = is_white ? board.get_white_king() : board.get_black_king();
        if (king_bb == 0) return;
        int king_sq = first_set_bit(king_bb);
        if (king_sq != (is_white ? 4 : 60)) return;

        uint64_t occ = board.get_all_occupancy();
        uint64_t own_rooks = is_white ? board.get_white_rooks() : board.get_black_rooks();

        if (is_in_check(board, is_white)) return;

        if (rights & king_side_flag) {
            int rook_sq = is_white ? 7 : 63;
            int f_sq = king_sq + 1, g_sq = king_sq + 2;
            bool rook_present = own_rooks & (1ULL << rook_sq);
            bool squares_empty = !(occ & ((1ULL << f_sq) | (1ULL << g_sq)));
            bool path_safe = !is_square_attacked(board, f_sq, !is_white) &&
                              !is_square_attacked(board, g_sq, !is_white);
            if (rook_present && squares_empty && path_safe) {
                Move m;
                m.from = static_cast<uint8_t>(king_sq);
                m.to = static_cast<uint8_t>(g_sq);
                m.is_castle_kingside = true;
                moves.push_back(m);
            }
        }

        if (rights & queen_side_flag) {
            int rook_sq = is_white ? 0 : 56;
            int d_sq = king_sq - 1, c_sq = king_sq - 2, b_sq = king_sq - 3;
            bool rook_present = own_rooks & (1ULL << rook_sq);
            bool squares_empty = !(occ & ((1ULL << d_sq) | (1ULL << c_sq) | (1ULL << b_sq)));
            bool path_safe = !is_square_attacked(board, d_sq, !is_white) &&
                              !is_square_attacked(board, c_sq, !is_white);
            if (rook_present && squares_empty && path_safe) {
                Move m;
                m.from = static_cast<uint8_t>(king_sq);
                m.to = static_cast<uint8_t>(c_sq);
                m.is_castle_queenside = true;
                moves.push_back(m);
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
    add_castling_moves(moves, board, is_white);

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
    Board scratch = board;
    for (const auto& m : generate_pseudo_legal_moves(board, is_white)) {
        UndoInfo undo = make_move(scratch, m);
        if (!is_in_check(scratch, is_white)) {
            legal.push_back(m);
        }
        unmake_move(scratch, m, undo);
    }
    return legal;
}

namespace {
    PieceCode promoted_piece_code(const Move& move, bool is_white) {
        switch (move.promotion) {
            case PromoPiece::QUEEN:  return is_white ? WQ : BQ;
            case PromoPiece::ROOK:   return is_white ? WR : BR;
            case PromoPiece::BISHOP: return is_white ? WB : BB;
            case PromoPiece::KNIGHT: return is_white ? WN : BN;
            default:                 return EMPTY;
        }
    }

    void castle_rook_squares(const Move& move, bool is_white, int& rook_from, int& rook_to) {
        if (move.is_castle_kingside) {
            rook_from = is_white ? 7 : 63;
            rook_to   = is_white ? 5 : 61;
        } else {
            rook_from = is_white ? 0 : 56;
            rook_to   = is_white ? 3 : 59;
        }
    }
}

UndoInfo make_move(Board& board, const Move& move) {
    UndoInfo undo;
    bool is_white = board.is_white_to_move();

    undo.prev_en_passant = board.get_en_passant_square();
    undo.prev_castling_rights = board.get_castling_rights();

    int8_t moving_piece = board.piece_at(move.from);
    undo.moving_piece = moving_piece;
    if (moving_piece == EMPTY) return undo;

    auto moving_pc = static_cast<PieceCode>(moving_piece);

    if (move.is_en_passant) {
        int captured_sq = is_white ? move.to - 8 : move.to + 8;
        PieceCode captured_pc = is_white ? BP : WP;
        undo.captured_piece = captured_pc;
        undo.captured_square = captured_sq;
        board.remove_piece(captured_pc, captured_sq);
    } else if (move.is_capture) {
        int8_t captured = board.piece_at(move.to);
        if (captured != EMPTY) {
            undo.captured_piece = captured;
            undo.captured_square = move.to;
            board.remove_piece(static_cast<PieceCode>(captured), move.to);
        }
    }

    board.remove_piece(moving_pc, move.from);

    PieceCode placed_pc = moving_pc;
    if (move.promotion != PromoPiece::NONE) {
        placed_pc = promoted_piece_code(move, is_white);
    }
    board.add_piece(placed_pc, move.to);

    if (move.is_castle_kingside || move.is_castle_queenside) {
        PieceCode rook_pc = is_white ? WR : BR;
        int rook_from, rook_to;
        castle_rook_squares(move, is_white, rook_from, rook_to);
        board.remove_piece(rook_pc, rook_from);
        board.add_piece(rook_pc, rook_to);
    }

    uint8_t rights = undo.prev_castling_rights;
    if (moving_piece == WK) rights &= ~(CASTLE_WK | CASTLE_WQ);
    if (moving_piece == BK) rights &= ~(CASTLE_BK | CASTLE_BQ);
    if (move.from == 0 || move.to == 0)   rights &= ~CASTLE_WQ;
    if (move.from == 7 || move.to == 7)   rights &= ~CASTLE_WK;
    if (move.from == 56 || move.to == 56) rights &= ~CASTLE_BQ;
    if (move.from == 63 || move.to == 63) rights &= ~CASTLE_BK;
    board.set_castling_rights(rights);

    board.set_en_passant_square(move.is_double_push ? (is_white ? move.to - 8 : move.to + 8) : -1);
    board.set_white_to_move(!is_white);

    return undo;
}

void unmake_move(Board& board, const Move& move, const UndoInfo& undo) {
    bool is_white = !board.is_white_to_move();
    board.set_white_to_move(is_white);
    board.set_castling_rights(undo.prev_castling_rights);
    board.set_en_passant_square(undo.prev_en_passant);

    if (undo.moving_piece == EMPTY) return;

    if (move.is_castle_kingside || move.is_castle_queenside) {
        PieceCode rook_pc = is_white ? WR : BR;
        int rook_from, rook_to;
        castle_rook_squares(move, is_white, rook_from, rook_to);
        board.remove_piece(rook_pc, rook_to);
        board.add_piece(rook_pc, rook_from);
    }

    PieceCode placed_pc = static_cast<PieceCode>(undo.moving_piece);
    if (move.promotion != PromoPiece::NONE) {
        placed_pc = promoted_piece_code(move, is_white);
    }
    board.remove_piece(placed_pc, move.to);
    board.add_piece(static_cast<PieceCode>(undo.moving_piece), move.from);

    if (undo.captured_piece != EMPTY) {
        board.add_piece(static_cast<PieceCode>(undo.captured_piece), undo.captured_square);
    }
}