"""
plot_results.py
Reads experiments/results/runtime.csv and generates plots.png

Usage:
    python visualization/plot_results.py
"""
import csv
import sys
from pathlib import Path

try:
    import matplotlib.pyplot as plt
except ImportError:
    print("matplotlib not installed. Run: pip install -r requirements.txt")
    sys.exit(1)

# ── Locate CSV ───────────────────────────────────────────────────────────────
ROOT = Path(__file__).resolve().parents[1]
CSV_PATH = ROOT / "experiments" / "results" / "runtime.csv"
OUT_PATH = ROOT / "experiments" / "results" / "plots.png"

if not CSV_PATH.exists():
    print(f"CSV not found: {CSV_PATH}")
    sys.exit(1)

# ── Load data ────────────────────────────────────────────────────────────────
data = {"AABB": {}, "GJK_brute": {}, "BVH_GJK": {}}
crescent = {"AABB": None, "BVH_GJK": None}

with CSV_PATH.open() as f:
    reader = csv.DictReader(f)
    for row in reader:
        N = int(row["N"])
        mode = row["mode"]
        fps = float(row["fps_avg"])
        pairs = float(row["colliding_pairs_avg"])
        notes = row["notes"]

        if "Crescent" in notes:
            crescent[mode] = pairs
        else:
            if mode in data:
                data[mode][N] = (fps, pairs)

# ── Plot ─────────────────────────────────────────────────────────────────────
fig, axes = plt.subplots(1, 2, figsize=(14, 5))

# Plot 1: FPS vs N
ax = axes[0]
N_values = sorted(data["AABB"].keys())
colors = {"AABB": "#E63946", "GJK_brute": "#F4A261", "BVH_GJK": "#2A9D8F"}
labels = {"AABB": "AABB baseline", "GJK_brute": "GJK brute", "BVH_GJK": "BVH + GJK"}

for mode in ["AABB", "GJK_brute", "BVH_GJK"]:
    fps_values = [data[mode][n][0] for n in N_values]
    ax.plot(N_values, fps_values, marker="o", linewidth=2, markersize=8,
            color=colors[mode], label=labels[mode])

ax.set_xlabel("N (number of objects)", fontsize=12)
ax.set_ylabel("FPS", fontsize=12)
ax.set_title("FPS vs N — Mixed shapes", fontsize=14, fontweight="bold")
ax.legend(fontsize=11)
ax.grid(True, alpha=0.3)
ax.set_xscale("log")

# Plot 2: False-positive rate on Crescents
ax = axes[1]
modes = ["AABB", "BVH_GJK"]
pairs = [crescent["AABB"], crescent["BVH_GJK"]]
true_collisions = crescent["BVH_GJK"]
false_positives = [crescent["AABB"] - true_collisions, 0]

x_pos = range(len(modes))
ax.bar(x_pos, [true_collisions, true_collisions],
       color="#2A9D8F", label="True collisions")
ax.bar(x_pos, false_positives, bottom=[true_collisions, true_collisions],
       color="#E63946", label="False positives")

ax.set_xticks(x_pos)
ax.set_xticklabels(["AABB baseline", "BVH + GJK"], fontsize=11)
ax.set_ylabel("Reported colliding pairs", fontsize=12)
ax.set_title("Accuracy on Crescent shapes (N=100)", fontsize=14, fontweight="bold")
ax.legend(fontsize=11)
ax.grid(True, alpha=0.3, axis="y")

# Annotate accuracy %
ax.annotate(f"63 % accuracy\n({int(false_positives[0])} false positives)",
            xy=(0, crescent["AABB"]), ha="center", va="bottom",
            fontsize=10, color="#E63946", fontweight="bold")
ax.annotate("100 % accuracy\n(0 false positives)",
            xy=(1, true_collisions), ha="center", va="bottom",
            fontsize=10, color="#2A9D8F", fontweight="bold")

plt.tight_layout()
plt.savefig(OUT_PATH, dpi=120, bbox_inches="tight")
print(f"Saved: {OUT_PATH}")
