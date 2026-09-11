#include "board.hpp"
#include <bit>
#include <cctype>

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

void Board::set_from_fen(const std::string& fen) {
    bitboards.fill(0);

    int rank = 7, file = 0;
    size_t i = 0;

    for (; i < fen.size() && fen[i] != ' '; ++i) {
        char c = fen[i];
        if (c == '/') {
            rank--;
            file = 0;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            file += c - '0';
        } else {
            int square = rank * 8 + file;
            PieceCode pc = EMPTY;
            switch (c) {
                case 'P': pc = WP; break; case 'N': pc = WN; break;
                case 'B': pc = WB; break; case 'R': pc = WR; break;
                case 'Q': pc = WQ; break; case 'K': pc = WK; break;
                case 'p': pc = BP; break; case 'n': pc = BN; break;
                case 'b': pc = BB; break; case 'r': pc = BR; break;
                case 'q': pc = BQ; break; case 'k': pc = BK; break;
            }
            if (pc != EMPTY) bitboards[pc] |= (1ULL << square);
            file++;
        }
    }

    // Side to move.
    while (i < fen.size() && fen[i] == ' ') i++;
    white_to_move = (i < fen.size() && fen[i] == 'w');
    while (i < fen.size() && fen[i] != ' ') i++;

    // Castling rights. A missing field or '-' means no castling; anything not
    // listed is cleared, so a FEN can never leave a stale constructor default.
    while (i < fen.size() && fen[i] == ' ') i++;
    castling_rights = 0;
    for (; i < fen.size() && fen[i] != ' '; ++i) {
        switch (fen[i]) {
            case 'K': castling_rights |= CASTLE_WK; break;
            case 'Q': castling_rights |= CASTLE_WQ; break;
            case 'k': castling_rights |= CASTLE_BK; break;
            case 'q': castling_rights |= CASTLE_BQ; break;
            default: break;  // '-' or unrecognised
        }
    }

    // En passant target square (e.g. "e3"), or '-' when there is none.
    while (i < fen.size() && fen[i] == ' ') i++;
    en_passant_square = -1;
    if (i < fen.size() && fen[i] != ' ' && fen[i] != '-') {
        char ep_file = fen[i];
        char ep_rank = (i + 1 < fen.size()) ? fen[i + 1] : '\0';
        if (ep_file >= 'a' && ep_file <= 'h' && ep_rank >= '1' && ep_rank <= '8') {
            en_passant_square = (ep_rank - '1') * 8 + (ep_file - 'a');
        }
    }

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