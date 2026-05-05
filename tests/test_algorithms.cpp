// tests/test_algorithms.cpp
// Simple unit tests for GJK and BVH (no external test framework — keeps deps minimal)

#include "math2d.h"
#include "convex_hull.h"
#include "gjk.h"
#include "bvh.h"
#include "object.h"
#include <cassert>
#include <cstdio>
#include <vector>

#define TEST(name) \
    do { printf("  [TEST] " name "\n"); } while (0)
#define CHECK(cond, msg) \
    do { if (!(cond)) { printf("    FAIL: " msg "\n"); failed++; } else { passed++; } } while (0)

int passed = 0, failed = 0;

// ── GJK Tests ─────────────────────────────────────────────────────────────
void test_gjk_separated_boxes() {
    TEST("Two separated boxes should NOT collide");
    std::vector<Vec2> A = {{0,0},{10,0},{10,10},{0,10}};
    std::vector<Vec2> B = {{20,0},{30,0},{30,10},{20,10}};
    CHECK(!gjkIntersect(A, B), "Should report no collision");
}

void test_gjk_overlapping_boxes() {
    TEST("Two overlapping boxes should collide");
    std::vector<Vec2> A = {{0,0},{10,0},{10,10},{0,10}};
    std::vector<Vec2> B = {{5,5},{15,5},{15,15},{5,15}};
    CHECK(gjkIntersect(A, B), "Should report collision");
}

void test_gjk_identical_shapes() {
    TEST("Two identical shapes at same position should collide");
    std::vector<Vec2> A = {{0,0},{10,0},{10,10},{0,10}};
    CHECK(gjkIntersect(A, A), "Identical shapes must collide");
}

void test_gjk_touching_boundary() {
    TEST("Two boxes sharing an edge — boundary case");
    std::vector<Vec2> A = {{0,0},{10,0},{10,10},{0,10}};
    std::vector<Vec2> B = {{10,0},{20,0},{20,10},{10,10}};
    // Implementation-dependent; this just checks no crash.
    bool result = gjkIntersect(A, B);
    CHECK(true, "no crash on boundary case");
    (void)result;
}

// ── Convex Hull Tests ─────────────────────────────────────────────────────
void test_hull_triangle() {
    TEST("Hull of 3 non-collinear points = the 3 points");
    std::vector<Vec2> pts = {{0,0},{10,0},{5,10}};
    auto hull = convexHull(pts);
    CHECK(hull.size() == 3, "Hull should have 3 vertices");
}

void test_hull_with_interior_point() {
    TEST("Hull of square + 1 interior point = 4 vertices");
    std::vector<Vec2> pts = {{0,0},{10,0},{10,10},{0,10},{5,5}};
    auto hull = convexHull(pts);
    CHECK(hull.size() == 4, "Interior point should be excluded");
}

void test_hull_collinear() {
    TEST("Hull of collinear points = 2 endpoints");
    std::vector<Vec2> pts = {{0,0},{5,5},{10,10},{2,2}};
    auto hull = convexHull(pts);
    CHECK(hull.size() <= 2, "Collinear points should reduce to endpoints");
}

// ── AABB Tests ────────────────────────────────────────────────────────────
void test_aabb_overlap() {
    TEST("AABB overlap detection");
    AABB a{0, 0, 10, 10};
    AABB b{5, 5, 15, 15};
    AABB c{20, 20, 30, 30};
    CHECK(a.overlaps(b),  "Overlapping AABBs");
    CHECK(!a.overlaps(c), "Non-overlapping AABBs");
}

// ── BVH Tests ─────────────────────────────────────────────────────────────
void test_bvh_build() {
    TEST("BVH build on 5 objects");
    std::vector<Object> objs;
    for (int i = 0; i < 5; i++) {
        objs.push_back(Object::makeBox({(double)(i*30), 100}, {0,0}, 20, 20));
    }
    BVH bvh;
    bvh.build(objs);
    CHECK(bvh.rootIdx >= 0, "BVH should have valid root");
    CHECK(bvh.nodes.size() >= 5, "BVH must contain at least N leaves");
}

void test_bvh_query_no_overlap() {
    TEST("BVH query on well-separated objects = 0 pairs");
    std::vector<Object> objs;
    for (int i = 0; i < 5; i++) {
        objs.push_back(Object::makeBox({(double)(i*100), 100}, {0,0}, 10, 10));
    }
    BVH bvh;
    bvh.build(objs);
    std::vector<std::pair<int,int>> pairs;
    bvh.queryPairs(pairs);
    CHECK(pairs.empty(), "Far-apart objects should yield no candidate pairs");
}

// ── Main ──────────────────────────────────────────────────────────────────
int main() {
    printf("Running GJK + BVH unit tests...\n\n");

    test_gjk_separated_boxes();
    test_gjk_overlapping_boxes();
    test_gjk_identical_shapes();
    test_gjk_touching_boundary();

    test_hull_triangle();
    test_hull_with_interior_point();
    test_hull_collinear();

    test_aabb_overlap();

    test_bvh_build();
    test_bvh_query_no_overlap();

    printf("\n────────────────────────────\n");
    printf("Passed: %d   Failed: %d\n", passed, failed);
    printf("────────────────────────────\n");
    return failed == 0 ? 0 : 1;
}
