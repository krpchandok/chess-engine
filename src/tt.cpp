#include "tt.hpp"

namespace {
    size_t round_down_pow2(size_t n) {
        size_t p = 1;
        while (p * 2 <= n) p *= 2;
        return p;
    }
}

TranspositionTable::TranspositionTable(size_t num_entries) {
    size_t size = round_down_pow2(num_entries == 0 ? 1 : num_entries);
    entries_.resize(size);
    mask_ = size - 1;
}

void TranspositionTable::store(uint64_t key, int depth, int score, TTFlag flag, const Move& best_move) {
    size_t idx = index_for(key);
    std::lock_guard<std::mutex> lock(lock_for(idx));

    TTEntry& slot = entries_[idx];
    if (depth >= slot.depth) {
        slot.key = key;
        slot.depth = depth;
        slot.score = score;
        slot.flag = flag;
        slot.best_move = best_move;
    }
}

bool TranspositionTable::probe(uint64_t key, TTEntry& out) const {
    size_t idx = index_for(key);
    std::lock_guard<std::mutex> lock(lock_for(idx));

    const TTEntry& slot = entries_[idx];
    if (slot.depth >= 0 && slot.key == key) {
        out = slot;
        return true;
    }
    return false;
}

void TranspositionTable::clear() {
    for (size_t i = 0; i < entries_.size(); ++i) {
        std::lock_guard<std::mutex> lock(lock_for(i));
        entries_[i] = TTEntry{};
    }
}