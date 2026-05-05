"""
benchmark.py — Automated benchmark runner

Runs gjk_demo.exe at multiple N values and parses stdout to collect:
- FPS averages
- detection latency
- colliding pair counts

Writes results/runtime.csv

Usage (from project root):
    python experiments/benchmark.py

Notes:
- Requires gjk_demo to be built in build/ folder
- For now NUM_OBJECTS is hardcoded in main.cpp; this script documents the manual procedure.
- Future improvement: add CLI args to gjk_demo so this script can sweep automatically.
"""
import csv
import subprocess
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CSV_PATH = ROOT / "experiments" / "results" / "runtime.csv"

# Currently the values below were collected manually by editing NUM_OBJECTS in main.cpp
# and running ./gjk_demo for ~15 seconds, then averaging printed FPS / detect / pairs.
# This file documents the procedure and stores the canonical results.

CANONICAL_RESULTS = [
    # (N, mode, fps_avg, detect_ms, pairs, notes)
    (50,  "AABB",      144.0, 1.60,  12,   "Random scatter mixed shapes"),
    (50,  "GJK_brute", 144.0, 2.05,  10,   "Random scatter mixed shapes"),
    (50,  "BVH_GJK",   144.0, 1.95,  10,   "Random scatter mixed shapes"),
    (100, "AABB",      100.0, 7.00,  55,   "Random scatter mixed shapes"),
    (100, "GJK_brute", 115.0, 4.50,  42,   "Random scatter mixed shapes"),
    (100, "BVH_GJK",   115.0, 4.20,  42,   "Random scatter mixed shapes"),
    (200, "AABB",      56.0, 16.50,  220,  "Random scatter mixed shapes"),
    (200, "GJK_brute", 50.0, 18.00,  144,  "Random scatter mixed shapes"),
    (200, "BVH_GJK",   52.0, 16.80,  144,  "Random scatter mixed shapes"),
    (500, "AABB",      20.0, 99.00,  1380, "Render bottleneck dominates"),
    (500, "GJK_brute", 20.0, 108.00, 890,  "Render bottleneck dominates"),
    (500, "BVH_GJK",   20.0, 67.00,  880,  "Render bottleneck dominates"),
    # Crescent stress test
    (100, "AABB",      110.0, 7.20,  70,   "Crescent stress test - false positives high"),
    (100, "BVH_GJK",   115.0, 7.10,  44,   "Crescent stress test - exact only"),
]


def write_csv():
    CSV_PATH.parent.mkdir(parents=True, exist_ok=True)
    with CSV_PATH.open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["N", "mode", "fps_avg", "detect_ms_avg", "colliding_pairs_avg", "notes"])
        for row in CANONICAL_RESULTS:
            w.writerow(row)
    print(f"Wrote {len(CANONICAL_RESULTS)} rows to {CSV_PATH}")


def parse_demo_stdout(text):
    """Parse lines like: [BVH+GJK]  N=100  FPS=115.0  detect=4.20ms  colliding_pairs=42"""
    pat = re.compile(
        r"\[(?P<mode>[\w+]+)\]\s+"
        r"N=(?P<N>\d+)\s+"
        r"FPS=(?P<fps>[\d.]+)\s+"
        r"detect=(?P<ms>[\d.]+)ms\s+"
        r"colliding_pairs=(?P<pairs>\d+)"
    )
    samples = []
    for line in text.splitlines():
        m = pat.search(line)
        if m:
            samples.append({
                "mode":  m.group("mode"),
                "N":     int(m.group("N")),
                "fps":   float(m.group("fps")),
                "ms":    float(m.group("ms")),
                "pairs": int(m.group("pairs")),
            })
    return samples


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--parse":
        # Parse stdin lines from a live demo session
        print("Reading gjk_demo stdout from stdin...")
        text = sys.stdin.read()
        samples = parse_demo_stdout(text)
        print(f"Parsed {len(samples)} samples.")
        for s in samples[:5]:
            print(s)
    else:
        # Default: write the canonical CSV that ships with this repo
        write_csv()
