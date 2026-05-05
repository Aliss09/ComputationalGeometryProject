# Demo — GJK Collision Detection

This folder contains the project demo materials.

## Type: Video Demo (Type A)

A 1-2 minute video showing the algorithm in action.

## How to view

1. Open `video_link.txt` for the YouTube link
2. Or watch the screenshots in `screenshots/` for a static walkthrough

## What the demo shows

The video covers all three required elements:

1. **Input** — Mixed Non-Convex shapes (stars, L-shapes, crescents) at N=100
2. **Algorithm in action** — Live SDL2 rendering with FPS counter, toggling between AABB baseline and BVH+GJK with the G key
3. **Output** — Colliding pairs highlighted in red, with detection time and pair count printed to terminal

## Reproducing the demo locally

If you want to record your own demo, build the project and run:

```bash
./gjk_demo
```

Then press `G` to switch modes during runtime.

## Backup screenshots

If the video link is unavailable, see `screenshots/` for static images of:
- AABB baseline mode (showing high false-positive rate)
- BVH+GJK mode (showing exact collisions only)
- Comparison side-by-side
