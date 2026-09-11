#include "nnue_eval.hpp"
#include <fstream>
#include <cmath>
#include <bit>

void extract_features(const Board& board, std::array<float, NNUE_INPUT_SIZE>& out) {
    out.fill(0.0f);
    for (int pc = 0; pc < 12; ++pc) {
        uint64_t bb = board.get_bitboard(static_cast<PieceCode>(pc));
        while (bb != 0) {
            int square = 0;
            while ((bb & 1ULL) == 0) {
                bb >>= 1;
                ++square;
            }
            bb &= bb - 1;
            out[pc * 64 + square] = 1.0f;
        }
    }
}

namespace {
    // Standard ReLU used between hidden layers.
    inline float relu(float x) { return x > 0.0f ? x : 0.0f; }

    template <size_t N>
    bool read_array(std::ifstream& in, std::array<float, N>& arr) {
        in.read(reinterpret_cast<char*>(arr.data()), sizeof(float) * N);
        return static_cast<bool>(in);
    }
}

bool load_nnue_weights(const std::string& path, NNUEWeights& weights) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        weights.loaded = false;
        return false;
    }

    // Layout must exactly match the order train_nnue.py writes in
    // export_weights(): w1 (row-major, HIDDEN1 x INPUT), b1, w2
    // (row-major, HIDDEN2 x HIDDEN1), b2, w3 (HIDDEN2), b3 (scalar).
    for (auto& row : weights.w1) {
        if (!read_array(in, row)) { weights.loaded = false; return false; }
    }
    if (!read_array(in, weights.b1)) { weights.loaded = false; return false; }

    for (auto& row : weights.w2) {
        if (!read_array(in, row)) { weights.loaded = false; return false; }
    }
    if (!read_array(in, weights.b2)) { weights.loaded = false; return false; }

    if (!read_array(in, weights.w3)) { weights.loaded = false; return false; }

    in.read(reinterpret_cast<char*>(&weights.b3), sizeof(float));
    if (!in) { weights.loaded = false; return false; }

    weights.loaded = true;
    return true;
}

int evaluate_nnue(const Board& board, const NNUEWeights& weights) {
    if (!weights.loaded) return 0;

    std::array<float, NNUE_INPUT_SIZE> features;
    extract_features(board, features);

    std::array<float, NNUE_HIDDEN1_SIZE> h1;
    for (int i = 0; i < NNUE_HIDDEN1_SIZE; ++i) {
        float sum = weights.b1[i];
        for (int j = 0; j < NNUE_INPUT_SIZE; ++j) {
            sum += weights.w1[i][j] * features[j];
        }
        h1[i] = relu(sum);
    }

    std::array<float, NNUE_HIDDEN2_SIZE> h2;
    for (int i = 0; i < NNUE_HIDDEN2_SIZE; ++i) {
        float sum = weights.b2[i];
        for (int j = 0; j < NNUE_HIDDEN1_SIZE; ++j) {
            sum += weights.w2[i][j] * h1[j];
        }
        h2[i] = relu(sum);
    }

    float out = weights.b3;
    for (int i = 0; i < NNUE_HIDDEN2_SIZE; ++i) {
        out += weights.w3[i] * h2[i];
    }

    // Trained to regress centipawn-scale search scores directly,
    // so no extra scaling needed here.
    return static_cast<int>(std::lround(out));
}