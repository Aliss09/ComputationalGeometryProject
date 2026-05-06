#include <SDL2/SDL.h>
#include <vector>
#include <chrono>
#include <string>
#include <random>
#include <algorithm>
#include <cstdio>

#include "math2d.h"
#include "object.h"
#include "gjk.h"
#include "bvh.h"

// ─── Config ──────────────────────────────────────────────────────────────────
static constexpr int   SCREEN_W   = 900;
static constexpr int   SCREEN_H   = 700;
static constexpr int   NUM_OBJECTS = 100;  // change to 50,100,200,500 for benchmark
static constexpr bool  USE_GJK    = true; // false = AABB brute-force baseline
static constexpr bool  USE_BVH    = true;  // ใช้ BVH+GJK แทน brute-force GJK

// ─── Renderer helpers ─────────────────────────────────────────────────────────
void drawPolygon(SDL_Renderer* r, const std::vector<Vec2>& pts, SDL_Color col) {
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    int n = pts.size();
    for (int i = 0; i < n; i++) {
        int j = (i+1) % n;
        SDL_RenderDrawLine(r,
            (int)pts[i].x, (int)pts[i].y,
            (int)pts[j].x, (int)pts[j].y);
    }
}

void drawAABB(SDL_Renderer* r, const AABB& b, SDL_Color col) {
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_Rect rect{(int)b.minX, (int)b.minY,
                  (int)(b.maxX-b.minX), (int)(b.maxY-b.minY)};
    SDL_RenderDrawRect(r, &rect);
}

// ─── Collision detection modes ────────────────────────────────────────────────
// BASELINE: All-pairs AABB O(N²)
int detectBaseline(std::vector<Object>& objs) {
    int count = 0;
    for (auto& o : objs) o.colliding = false;
    for (int i = 0; i < (int)objs.size(); i++) {
        for (int j = i+1; j < (int)objs.size(); j++) {
            if (objs[i].aabb().overlaps(objs[j].aabb())) {
                objs[i].colliding = objs[j].colliding = true;
                count++;
            }
        }
    }
    return count;
}

// IMPROVEMENT: AABB broadphase → GJK narrowphase
int detectGJK(std::vector<Object>& objs) {
    int count = 0;
    for (auto& o : objs) o.colliding = false;
    for (int i = 0; i < (int)objs.size(); i++) {
        for (int j = i+1; j < (int)objs.size(); j++) {
            // Broadphase: fast AABB reject
            if (!objs[i].aabb().overlaps(objs[j].aabb())) continue;
            // Narrowphase: exact GJK on convex pieces
            auto pa = objs[i].worldPieces();
            auto pb = objs[j].worldPieces();
            if (gjkNonConvex(pa, pb)) {
                objs[i].colliding = objs[j].colliding = true;
                count++;
            }
        }
    }
    return count;
}
// IMPROVEMENT v2: BVH broadphase → GJK narrowphase
int detectBVH(std::vector<Object>& objs, BVH& bvh) {
    int count = 0;
    for (auto& o : objs) o.colliding = false;

    bvh.build(objs);
    std::vector<std::pair<int,int>> pairs;
    bvh.queryPairs(pairs);

    for (auto& [i, j] : pairs) {
        auto pa = objs[i].worldPieces();
        auto pb = objs[j].worldPieces();
        if (gjkNonConvex(pa, pb)) {
            objs[i].colliding = objs[j].colliding = true;
            count++;
        }
    }
    return count;
}
// ─── Main ─────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*   win = SDL_CreateWindow(
        "GJK Non-Convex Collision  |  ESC=quit  |  G=toggle mode  |  +/-=objects",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // ── Spawn objects ──────────────────────────────────────────────────────
    std::mt19937 rng(42);
    auto randu = [&](double lo, double hi) {
        return lo + (hi-lo) * (rng() / (double)rng.max());
    };

    std::vector<Object> objs;
    objs.reserve(NUM_OBJECTS);
    for (int i = 0; i < NUM_OBJECTS; i++) {
        double cx = randu(60, SCREEN_W-60);
        double cy = randu(60, SCREEN_H-60);
        double vx = randu(-80, 80);
        double vy = randu(-80, 80);
        int type = i % 3;
        if      (type == 0) objs.push_back(Object::makeStar({cx,cy},{vx,vy}, 24, 10, 5));
        else if (type == 1) objs.push_back(Object::makeL({cx,cy},{vx,vy}, 36));
        else if (type == 2) objs.push_back(Object::makeCrescent({cx,cy},{vx,vy}, 28));
        else                objs.push_back(Object::makeBox({cx,cy},{vx,vy}, 32, 24));

        objs.back().angVel = randu(-1.5, 1.5);
    }

    // ── State ─────────────────────────────────────────────────────────────
    bool useGJK = USE_GJK;
    bool running = true;
    SDL_Event evt;

    using Clock = std::chrono::high_resolution_clock;
    auto prev = Clock::now();

    double fpsAccum  = 0;
    int    fpsFrames = 0;
    double displayFPS = 0;

    // Colors
    SDL_Color colNormal   = {100, 180, 255, 255};
    SDL_Color colCollide  = {255,  80,  80, 255};
    SDL_Color colAABB     = { 60,  60,  60, 180};

    // ── Main loop ─────────────────────────────────────────────────────────
    while (running) {
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) running = false;
            if (evt.type == SDL_KEYDOWN) {
                if (evt.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (evt.key.keysym.sym == SDLK_g)      useGJK = !useGJK;
            }
        }

        auto now = Clock::now();
        double dt = std::chrono::duration<double>(now - prev).count();
        prev = now;
        dt = std::min(dt, 0.05); // cap at 50ms

        // ── Update ────────────────────────────────────────────────────────
        for (auto& o : objs) o.update(dt, SCREEN_W, SCREEN_H);

        // ── Collision detection ───────────────────────────────────────────
        auto tStart = Clock::now();
        static BVH bvh;
        int pairs;
        if (useGJK && USE_BVH) pairs = detectBVH(objs, bvh);
        else if (useGJK)       pairs = detectGJK(objs);
        else                   pairs = detectBaseline(objs);
        double detectMs = std::chrono::duration<double,std::milli>(
            Clock::now() - tStart).count();

        // ── FPS ───────────────────────────────────────────────────────────
        fpsAccum += dt; fpsFrames++;
        if (fpsAccum >= 0.3) {
            displayFPS = fpsFrames / fpsAccum;
            fpsAccum = fpsFrames = 0;
        }

        // ── Render ────────────────────────────────────────────────────────
        SDL_SetRenderDrawColor(ren, 16, 16, 24, 255);
        SDL_RenderClear(ren);

        for (auto& o : objs) {
            SDL_Color col = o.colliding ? colCollide : colNormal;
            drawPolygon(ren, o.worldShape(), col);
            drawAABB(ren, o.aabb(), colAABB);
        }

        // ── HUD ───────────────────────────────────────────────────────────
        // (Simple text via SDL_RenderDrawLine pixel font workaround)
        // For real text rendering add SDL_ttf — keeping zero extra deps here.
        // FPS and mode are printed to stdout for benchmarking:
        static int frame = 0;
        if (frame++ % 30 == 0) {
            printf("[%s]  N=%d  FPS=%.1f  detect=%.2fms  colliding_pairs=%d\n",
                    (useGJK && USE_BVH) ? "BVH+GJK" : (useGJK ? "GJK" : "AABB"),
                   (int)objs.size(),
                   displayFPS,
                   detectMs,
                   pairs);
            fflush(stdout);
        }

        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
