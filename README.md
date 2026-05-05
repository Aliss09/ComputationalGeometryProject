# GJK Collision Detection on Non-Convex Shapes

> **Computational Geometry — Final Project · Term 2/2568**
> Real-time collision detection for Non-Convex polygons via Convex Decomposition + BVH + GJK

---

## Problem

Game physics needs fast and accurate collision detection between many moving objects. The classical GJK algorithm (Gilbert–Johnson–Keerthi 1988) is exact and fast — but only works on **Convex Shapes**. Real game objects (stars, L-shaped buildings, crescents) are **Non-Convex** and break GJK directly.

---

## Approach

- **Baseline:** All-Pairs AABB test  *O(N²)* — high false-positive rate on Non-Convex shapes
- **Proposed:** 3-phase pipeline *O(N log N + K · p²)*
  1. **Convex Decomposition** (preprocess) — fan triangulation per object
  2. **BVH Broadphase** — top-down split, *O(log N + K)* query
  3. **GJK Narrowphase** — exact collision via Minkowski difference

---

## Results

| Metric | Baseline (AABB) | Proposed (BVH+GJK) | Improvement |
|--------|----------------:|-------------------:|------------:|
| Accuracy on Crescents | 63 % | **100 %** | **+37 %** |
| FPS at N=100 | 100 | **115** | +15 % |
| False-positive rate | ~37 % | **0 %** | eliminated |

📺 **Video Demo:** [link in `demo/video_link.txt`]
📊 **Full benchmarks:** [`experiments/results/runtime.csv`](experiments/results/runtime.csv)
📄 **Report:** [`report/final_report.pdf`](report/final_report.pdf)

---

## How to Run

### Prerequisites
- C++17 compiler (GCC/Clang/MSVC)
- CMake 3.16+
- SDL2

### Windows (MSYS2 UCRT64)
```bash
pacman -S mingw-w64-ucrt-x86_64-{gcc,cmake,make,SDL2}
```

### Linux/macOS
```bash
sudo apt install libsdl2-dev cmake g++   # Ubuntu/Debian
brew install sdl2 cmake                  # macOS
```

### Build & Run
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"   # Windows
cmake ..                         # Linux/macOS
mingw32-make                     # Windows
make                             # Linux/macOS
./gjk_demo
```

### Controls

| Key | Action |
|-----|--------|
| `G` | Toggle AABB ↔ GJK ↔ BVH+GJK |
| `ESC` | Quit |

FPS, detection latency, and colliding pair counts print to stdout every 30 frames.

---

## Reproducing Benchmarks

```bash
cd experiments
python benchmark.py        # runs all N values, writes results/runtime.csv
python ../visualization/plot_results.py   # generates plots.png
```

All experiments use **fixed RNG seed = 42** for deterministic results.

---

## Project Structure

```
computational-geometry-project/
├── README.md                          ← you are here
├── report/
│   └── final_report.pdf               ← 8-page IEEE-format report
├── src/
│   ├── main.cpp                       ← SDL2 main loop
│   ├── algorithms/
│   │   ├── baseline.h                 ← All-pairs AABB
│   │   ├── proposed.h                 ← BVH + GJK pipeline
│   │   └── gjk.h                      ← Core GJK algorithm
│   └── data_structures/
│       ├── math2d.h                   ← Vec2, AABB
│       ├── bvh.h                      ← Bounding Volume Hierarchy
│       ├── object.h                   ← Game objects
│       └── convex_hull.h              ← Andrew's Monotone Chain
├── experiments/
│   ├── benchmark.py                   ← Sweep N, log results
│   └── results/
│       ├── runtime.csv                ← Raw measurements
│       └── plots.png                  ← FPS vs N graphs
├── data/
│   ├── synthetic/                     ← Generated test cases
│   └── real/                          ← (none for this project)
├── visualization/
│   └── plot_results.py                ← Matplotlib plotting
├── demo/
│   ├── README.md                      ← Demo instructions
│   ├── video_link.txt                 ← YouTube link
│   └── screenshots/                   ← Backup images
├── tests/
│   └── test_algorithms.cpp            ← Unit tests
├── CMakeLists.txt
└── requirements.txt                   ← Python deps for benchmark/plot
```

---

## License

Educational project. Open for learning and remixing.

---

## Author

**Aliss09** · GitHub: [@Aliss09](https://github.com/Aliss09)
Computational Geometry, Term 2/2568
