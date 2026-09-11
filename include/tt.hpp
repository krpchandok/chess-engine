#pragma once
#include <vector>
#include <mutex>
#include <array>
#include <cstdint>
#include "move.hpp"

enum class TTFlag : uint8_t { EXACT, LOWER, UPPER };

struct TTEntry {
    uint64_t key = 0;
    int depth = -1;
    int score = 0;
    TTFlag flag = TTFlag::EXACT;
    Move best_move{};
};

class TranspositionTable {
public:
    explicit TranspositionTable(size_t num_entries);

    void store(uint64_t key, int depth, int score, TTFlag flag, const Move& best_move);
    bool probe(uint64_t key, TTEntry& out) const;
    void clear();

private:
    static constexpr size_t NUM_SHARDS = 4096;

    std::vector<TTEntry> entries_;
    mutable std::array<std::mutex, NUM_SHARDS> shard_locks_;
    size_t mask_;

    size_t index_for(uint64_t key) const { return key & mask_; }
    std::mutex& lock_for(size_t index) const { return shard_locks_[index % NUM_SHARDS]; }
};