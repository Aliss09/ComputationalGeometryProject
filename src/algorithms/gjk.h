#pragma once
#include "math2d.h"
#include <vector>

// ─── Support Function ─────────────────────────────────────────────────────────
// Returns the point in 'shape' furthest in direction 'd'.
// This is the key function that makes GJK work on any convex shape.
inline Vec2 support(const std::vector<Vec2>& shape, const Vec2& d) {
    double best = -1e18;
    Vec2 result = shape[0];
    for (auto& v : shape) {
        double dot = v.dot(d);
        if (dot > best) { best = dot; result = v; }
    }
    return result;
}

// ─── Minkowski Difference Support ────────────────────────────────────────────
inline Vec2 minkSupport(const std::vector<Vec2>& A,
                         const std::vector<Vec2>& B,
                         const Vec2& d) {
    return support(A, d) - support(B, {-d.x, -d.y});
}

// ─── Simplex helpers (2D) ─────────────────────────────────────────────────────
struct Simplex {
    Vec2 pts[3];
    int  n = 0;

    void add(const Vec2& p) { pts[n++] = p; }
    void set(const Vec2& a)               { n=1; pts[0]=a; }
    void set(const Vec2& a, const Vec2& b){ n=2; pts[0]=a; pts[1]=b; }
};

// Returns true if simplex contains origin, updates d if not.
inline bool doSimplex(Simplex& s, Vec2& d) {
    if (s.n == 2) {
        // Line case
        Vec2 A = s.pts[1], B = s.pts[0];  // A = latest
        Vec2 AB = B - A, AO = Vec2{-A.x, -A.y};
        if (AB.dot(AO) > 0) {
            d = Vec2::tripleProduct(AB, AO, AB);
            if (d.norm2() < 1e-20) d = {-AB.y, AB.x}; // perp
        } else {
            s.set(A);
            d = AO;
        }
    } else {
        // Triangle case
        Vec2 A = s.pts[2], B = s.pts[1], C = s.pts[0];
        Vec2 AB = B-A, AC = C-A, AO = {-A.x,-A.y};
        Vec2 ABperp = Vec2::tripleProduct(AC, AB, AB);
        Vec2 ACperp = Vec2::tripleProduct(AB, AC, AC);

        if (ABperp.dot(AO) > 0) {
            s.set(B, A); d = ABperp;
        } else if (ACperp.dot(AO) > 0) {
            s.set(C, A); d = ACperp;
        } else {
            return true; // origin inside triangle → collision!
        }
    }
    return false;
}

// ─── GJK Collision Test ───────────────────────────────────────────────────────
// Returns true if convex shapes A and B intersect.
// This is the exact narrowphase test.
inline bool gjkIntersect(const std::vector<Vec2>& A,
                          const std::vector<Vec2>& B) {
    Vec2 d{1, 0};
    Simplex s;
    s.add(minkSupport(A, B, d));
    d = {-s.pts[0].x, -s.pts[0].y};

    for (int iter = 0; iter < 64; iter++) {
        if (d.norm2() < 1e-20) return true;
        Vec2 newPt = minkSupport(A, B, d);
        if (newPt.dot(d) < 0) return false; // No collision
        s.add(newPt);
        if (doSimplex(s, d)) return true;
    }
    return false; // convergence fallback
}

// ─── Non-Convex Collision via Decomposition ───────────────────────────────────
// Tests all piece pairs between two decomposed shapes.
// Returns true if any piece pair collides.
inline bool gjkNonConvex(const std::vector<std::vector<Vec2>>& piecesA,
                          const std::vector<std::vector<Vec2>>& piecesB) {
    for (auto& pa : piecesA)
        for (auto& pb : piecesB)
            if (gjkIntersect(pa, pb)) return true;
    return false;
}
