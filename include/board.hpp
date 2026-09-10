#pragma once
#include <cstdint>
#include <array>

enum PieceCode : int8_t {
    EMPTY = -1,
    WP, WN, WB, WR, WQ, WK,
    BP, BN, BB, BR, BQ, BK,
    PIECE_CODE_NB = 12
};

class Board {
    std::array<uint64_t, PIECE_CODE_NB> bitboards{};

    uint64_t white_occupancy = 0;
    uint64_t black_occupancy = 0;
    uint64_t all_occupancy = 0;

    std::array<int8_t, 64> mailbox{};

    int en_passant_square = -1;
    bool white_to_move = true;

public:
    Board();
    void initialize();
    void update_occupancy();
    void update_mailbox();

    uint64_t get_bitboard(PieceCode pc) const { return bitboards[pc]; }
    void set_bitboard(PieceCode pc, uint64_t bb) { bitboards[pc] = bb; }

    uint64_t get_white_occupancy() const { return white_occupancy; }
    uint64_t get_black_occupancy() const { return black_occupancy; }
    uint64_t get_all_occupancy() const { return all_occupancy; }

    int8_t piece_at(int square) const { return mailbox[square]; }

    int get_en_passant_square() const { return en_passant_square; }
    void set_en_passant_square(int sq) { en_passant_square = sq; }

    bool is_white_to_move() const { return white_to_move; }
    void set_white_to_move(bool w) { white_to_move = w; }

    uint64_t get_white_pawns()   const { return bitboards[WP]; }
    uint64_t get_white_knights() const { return bitboards[WN]; }
    uint64_t get_white_bishops() const { return bitboards[WB]; }
    uint64_t get_white_rooks()   const { return bitboards[WR]; }
    uint64_t get_white_queens()  const { return bitboards[WQ]; }
    uint64_t get_white_king()    const { return bitboards[WK]; }

    uint64_t get_black_pawns()   const { return bitboards[BP]; }
    uint64_t get_black_knights() const { return bitboards[BN]; }
    uint64_t get_black_bishops() const { return bitboards[BB]; }
    uint64_t get_black_rooks()   const { return bitboards[BR]; }
    uint64_t get_black_queens()  const { return bitboards[BQ]; }
    uint64_t get_black_king()    const { return bitboards[BK]; }
};