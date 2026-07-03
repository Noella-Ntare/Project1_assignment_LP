"""Benchmark: pure Python vs C extension for statistical computations."""

import sys
import time
import random

# ── Pure Python implementations ────────────────────────────────────────────

def py_full_stats(data):
    n = len(data)
    mean = sum(data) / n
    var = sum((x - mean) ** 2 for x in data) / n
    std = var ** 0.5
    mn = min(data)
    mx = max(data)
    return mean, std, mn, mx

# ── Import C extension ─────────────────────────────────────────────────────

try:
    import faststats
    HAS_C_EXT = True
except ImportError:
    HAS_C_EXT = False
    print("WARNING: C extension not found. Run: python3 setup.py build_ext --inplace")

# ── Benchmark helper ───────────────────────────────────────────────────────

def timeit(fn, *args, reps=3):
    best = float("inf")
    result = None
    for _ in range(reps):
        t0 = time.perf_counter()
        result = fn(*args)
        t1 = time.perf_counter()
        best = min(best, t1 - t0)
    return result, best

# ── Main ───────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    N = 1_000_000
    print(f"Generating {N:,} random floats...\n")
    data = [random.random() * 1000 for _ in range(N)]

    print("=" * 55)
    print(f"{'Benchmark':45s}  {'Time (s)':>8}")
    print("=" * 55)

    # Pure Python
    py_result, py_time = timeit(py_full_stats, data)
    print(f"{'Pure Python  full_stats()':45s}  {py_time:8.4f}")

    if HAS_C_EXT:
        c_result, c_time = timeit(faststats.full_stats, data)
        speedup = py_time / c_time
        print(f"{'C Extension  faststats.full_stats()':45s}  {c_time:8.4f}")
        print("=" * 55)
        print(f"\nSpeedup: {speedup:.1f}x faster with C extension\n")

        # Correctness check
        print("Correctness check (Python vs C extension):")
        for label, pv, cv in zip(
            ["mean", "std", "min", "max"],
            py_result, c_result
        ):
            match = abs(pv - cv) < 1e-9
            print(f"  {label:6s}: Python={pv:.6f}  C={cv:.6f}  {'OK' if match else 'MISMATCH'}")
    else:
        print("=" * 55)
        print("\nC extension not available; only Python result shown.")
        print(f"  mean={py_result[0]:.4f}, std={py_result[1]:.4f}")
        print(f"  min={py_result[2]:.4f},  max={py_result[3]:.4f}")
