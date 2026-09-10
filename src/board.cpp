#include "board.hpp"
#include <bit>

Board::Board() {
    initialize();
}

void Board::initialize() {
    bitboards[WP] = 0x000000000000FF00;
    bitboards[WN] = 0x0000000000000042;
    bitboards[WB] = 0x0000000000000024;
    bitboards[WR] = 0x0000000000000081;
    bitboards[WQ] = 0x0000000000000008;  // d1
    bitboards[WK] = 0x0000000000000010;  // e1

    bitboards[BP] = 0x00FF000000000000;
    bitboards[BN] = 0x4200000000000000;
    bitboards[BB] = 0x2400000000000000;
    bitboards[BR] = 0x8100000000000000;
    bitboards[BQ] = 0x0800000000000000;  // d8
    bitboards[BK] = 0x1000000000000000;  // e8

    en_passant_square = -1;
    white_to_move = true;

    update_occupancy();
    update_mailbox();
}

void Board::update_occupancy() {
    white_occupancy = bitboards[WP] | bitboards[WN] | bitboards[WB] |
                       bitboards[WR] | bitboards[WQ] | bitboards[WK];

    black_occupancy = bitboards[BP] | bitboards[BN] | bitboards[BB] |
                       bitboards[BR] | bitboards[BQ] | bitboards[BK];

    all_occupancy = white_occupancy | black_occupancy;
}

void Board::update_mailbox() {
    mailbox.fill(EMPTY);

    auto stamp = [this](uint64_t bb, int8_t code) {
        while (bb) {
            int square = 0;
            uint64_t scan = bb;
            while ((scan & 1ULL) == 0) {
                scan >>= 1;
                ++square;
            }
            mailbox[square] = code;
            bb &= bb - 1;
        }
    };

    for (int pc = WP; pc <= BK; ++pc) {
        stamp(bitboards[pc], static_cast<int8_t>(pc));
    }
}