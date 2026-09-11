"""
Trains the small MLP evaluator used by nnue_eval.hpp/.cpp.

Input: the text file produced by generate_selfplay_data (one line per
position: 768 space-separated 0/1 features, then a centipawn label).

Output: a raw binary weight file matching the layout read by
load_nnue_weights() in nnue_eval.cpp:
    w1 (256 x 768, row-major float32)
    b1 (256 float32)
    w2 (32 x 256, row-major float32)
    b2 (32 float32)
    w3 (32 float32)
    b3 (1 float32)

Usage:
    python train_nnue.py --data selfplay.txt --out nnue_weights.bin
"""
import argparse
import struct
import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader

INPUT_SIZE = 768
HIDDEN1_SIZE = 256
HIDDEN2_SIZE = 32


class PositionDataset(Dataset):
    def __init__(self, path):
        features = []
        labels = []
        with open(path) as f:
            for line in f:
                parts = line.split()
                if len(parts) != INPUT_SIZE + 1:
                    continue  # skip malformed lines
                features.append([float(x) for x in parts[:INPUT_SIZE]])
                labels.append(float(parts[INPUT_SIZE]))

        self.x = torch.tensor(features, dtype=torch.float32)
        # Clip extreme mate scores so they don't dominate the MSE loss;
        # centipawn values beyond a few queens' worth aren't meaningfully
        # different for training purposes.
        raw_labels = torch.tensor(labels, dtype=torch.float32)
        self.y = torch.clamp(raw_labels, -2000.0, 2000.0)

    def __len__(self):
        return len(self.y)

    def __getitem__(self, idx):
        return self.x[idx], self.y[idx]


class NNUEModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(INPUT_SIZE, HIDDEN1_SIZE)
        self.fc2 = nn.Linear(HIDDEN1_SIZE, HIDDEN2_SIZE)
        self.fc3 = nn.Linear(HIDDEN2_SIZE, 1)
        self.relu = nn.ReLU()

    def forward(self, x):
        h1 = self.relu(self.fc1(x))
        h2 = self.relu(self.fc2(h1))
        return self.fc3(h2).squeeze(-1)


def train(data_path, epochs, batch_size, lr):
    dataset = PositionDataset(data_path)
    n_val = max(1, int(0.1 * len(dataset)))
    n_train = len(dataset) - n_val
    train_set, val_set = torch.utils.data.random_split(dataset, [n_train, n_val])

    train_loader = DataLoader(train_set, batch_size=batch_size, shuffle=True)
    val_loader = DataLoader(val_set, batch_size=batch_size)

    model = NNUEModel()
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    loss_fn = nn.MSELoss()

    for epoch in range(epochs):
        model.train()
        total_loss = 0.0
        for xb, yb in train_loader:
            optimizer.zero_grad()
            pred = model(xb)
            loss = loss_fn(pred, yb)
            loss.backward()
            optimizer.step()
            total_loss += loss.item() * xb.size(0)
        train_mse = total_loss / n_train

        model.eval()
        val_loss = 0.0
        with torch.no_grad():
            for xb, yb in val_loader:
                pred = model(xb)
                val_loss += loss_fn(pred, yb).item() * xb.size(0)
        val_mse = val_loss / n_val

        print(f"epoch {epoch+1}/{epochs}  train_mse={train_mse:.1f}  "
              f"val_mse={val_mse:.1f}  val_rmse={val_mse**0.5:.1f}cp")

    return model


def export_weights(model, out_path):
    sd = model.state_dict()
    w1 = sd["fc1.weight"].detach().numpy().astype("<f4")  # (256, 768)
    b1 = sd["fc1.bias"].detach().numpy().astype("<f4")    # (256,)
    w2 = sd["fc2.weight"].detach().numpy().astype("<f4")  # (32, 256)
    b2 = sd["fc2.bias"].detach().numpy().astype("<f4")    # (32,)
    w3 = sd["fc3.weight"].detach().numpy().astype("<f4").reshape(-1)  # (32,)
    b3 = sd["fc3.bias"].detach().numpy().astype("<f4")    # (1,)

    assert w1.shape == (HIDDEN1_SIZE, INPUT_SIZE)
    assert w2.shape == (HIDDEN2_SIZE, HIDDEN1_SIZE)
    assert w3.shape == (HIDDEN2_SIZE,)

    with open(out_path, "wb") as f:
        f.write(w1.tobytes())
        f.write(b1.tobytes())
        f.write(w2.tobytes())
        f.write(b2.tobytes())
        f.write(w3.tobytes())
        f.write(struct.pack("<f", float(b3[0])))

    print(f"Wrote weights to {out_path} "
          f"({INPUT_SIZE}->{HIDDEN1_SIZE}->{HIDDEN2_SIZE}->1)")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data", required=True, help="path to self-play data file")
    parser.add_argument("--out", default="nnue_weights.bin")
    parser.add_argument("--epochs", type=int, default=30)
    parser.add_argument("--batch-size", type=int, default=256)
    parser.add_argument("--lr", type=float, default=1e-3)
    args = parser.parse_args()

    model = train(args.data, args.epochs, args.batch_size, args.lr)
    export_weights(model, args.out)


if __name__ == "__main__":
    main()