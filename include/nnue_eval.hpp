#pragma once
#include "board.hpp"
#include <array>
#include <string>

// A small NNUE-style evaluator: a fixed binary feature vector
// (one bit per piece-type/color/square) run through a tiny MLP
// trained offline in Python. This header exposes:
//   - feature extraction from a Board (shared by training data
//     generation and inference, so features never drift out of sync)
//   - loading trained weights from disk
//   - the forward-pass evaluation itself

constexpr int NNUE_INPUT_SIZE = 768;   // 12 piece codes * 64 squares
constexpr int NNUE_HIDDEN1_SIZE = 256;
constexpr int NNUE_HIDDEN2_SIZE = 32;

// Fills `out` with a 768-length 0/1 feature vector for `board`,
// always from White's perspective (label/eval convention matches
// the existing evaluate() function, which is also White-relative).
void extract_features(const Board& board, std::array<float, NNUE_INPUT_SIZE>& out);

struct NNUEWeights {
    // Layer 1: 768 -> 256
    std::array<std::array<float, NNUE_INPUT_SIZE>, NNUE_HIDDEN1_SIZE> w1;
    std::array<float, NNUE_HIDDEN1_SIZE> b1;
    // Layer 2: 256 -> 32
    std::array<std::array<float, NNUE_HIDDEN1_SIZE>, NNUE_HIDDEN2_SIZE> w2;
    std::array<float, NNUE_HIDDEN2_SIZE> b2;
    // Output layer: 32 -> 1
    std::array<float, NNUE_HIDDEN2_SIZE> w3;
    float b3 = 0.0f;

    bool loaded = false;
};

// Loads weights exported by train_nnue.py (see tools/). Returns false
// (and leaves weights.loaded == false) if the file can't be read.
bool load_nnue_weights(const std::string& path, NNUEWeights& weights);

// Forward pass. Returns a centipawn-scale score from White's perspective,
// matching the sign convention of evaluate() in eval.cpp.
int evaluate_nnue(const Board& board, const NNUEWeights& weights);