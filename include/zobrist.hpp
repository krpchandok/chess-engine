#pragma once
#include <cstdint>
#include "board.hpp"

void init_zobrist_keys();
uint64_t compute_zobrist_hash(const Board& board);