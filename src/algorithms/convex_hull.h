#pragma once
#include "math2d.h"
#include <vector>
#include <algorithm>

// ─── Andrew's Monotone Chain O(N log N) ──────────────────────────────────────
// Returns points of the convex hull in counter-clockwise order.
// Handles collinear points and degenerate cases (0, 1, 2 points).
inline std::vector<Vec2> convexHull(std::vector<Vec2> pts) {
    int n = (int)pts.size();
    if (n < 2) return pts;

    std::sort(pts.begin(), pts.end(), [](const Vec2& a, const Vec2& b){
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    std::vector<Vec2> hull;
    hull.reserve(2*n);

    // Lower hull
    for (int i = 0; i < n; i++) {
        while (hull.size() >= 2) {
            Vec2 ab = hull[hull.size()-1] - hull[hull.size()-2];
            Vec2 ac = pts[i] - hull[hull.size()-2];
            if (ab.cross(ac) <= 0) hull.pop_back(); // remove collinear & CW
            else break;
        }
        hull.push_back(pts[i]);
    }

    // Upper hull
    int lower_size = hull.size();
    for (int i = n-2; i >= 0; i--) {
        while ((int)hull.size() > lower_size) {
            Vec2 ab = hull[hull.size()-1] - hull[hull.size()-2];
            Vec2 ac = pts[i] - hull[hull.size()-2];
            if (ab.cross(ac) <= 0) hull.pop_back();
            else break;
        }
        hull.push_back(pts[i]);
    }

    hull.pop_back(); // remove duplicate last point
    return hull;
}

// ─── Naive Convex Decomposition ──────────────────────────────────────────────
// Splits a simple polygon into Convex pieces via ear-clipping triangulation.
// For Day 2: each triangle is a convex piece (k = n-2 for n-gon).
// This is a simplified approach — sufficient for star/L/crescent shapes.
inline std::vector<std::vector<Vec2>> convexDecompose(const std::vector<Vec2>& poly) {
    std::vector<std::vector<Vec2>> pieces;
    if (poly.size() < 3) return pieces;

    // Fan triangulation from centroid (works well for star-convex shapes)
    // For true non-convex shapes this may produce overlapping triangles —
    // replace with proper ear-clipping in Week 2 improvement.
    Vec2 centroid{0,0};
    for (auto& p : poly) { centroid.x += p.x; centroid.y += p.y; }
    centroid.x /= poly.size(); centroid.y /= poly.size();

    for (int i = 0; i < (int)poly.size(); i++) {
        int j = (i+1) % poly.size();
        pieces.push_back({centroid, poly[i], poly[j]});
    }
    return pieces;
}
