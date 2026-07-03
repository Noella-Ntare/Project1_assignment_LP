"""Pure Python implementation of statistical computations on large arrays."""

import time
import random

def compute_mean(data):
    """Compute arithmetic mean."""
    return sum(data) / len(data)

def compute_variance(data, mean):
    """Compute population variance."""
    return sum((x - mean) ** 2 for x in data) / len(data)

def compute_std(data):
    """Compute standard deviation."""
    mean = compute_mean(data)
    var = compute_variance(data, mean)
    return var ** 0.5

def compute_min_max(data):
    """Find minimum and maximum."""
    mn = data[0]
    mx = data[0]
    for x in data:
        if x < mn:
            mn = x
        if x > mx:
            mx = x
    return mn, mx

def full_stats(data):
    """Compute mean, std, min, max over a list."""
    mean = compute_mean(data)
    std = compute_std(data)
    mn, mx = compute_min_max(data)
    return mean, std, mn, mx


if __name__ == "__main__":
    N = 500_000
    print(f"Generating {N:,} random floats...")
    data = [random.random() * 1000 for _ in range(N)]

    print("Running pure Python stats computation...")
    t0 = time.perf_counter()
    mean, std, mn, mx = full_stats(data)
    t1 = time.perf_counter()

    print(f"  Mean : {mean:.4f}")
    print(f"  StdDev: {std:.4f}")
    print(f"  Min  : {mn:.4f}")
    print(f"  Max  : {mx:.4f}")
    print(f"  Time : {t1 - t0:.4f} seconds")
