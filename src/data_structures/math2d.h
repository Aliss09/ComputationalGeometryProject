#pragma once
#include <cmath>
#include <vector>

// ─── Vec2 ────────────────────────────────────────────────────────────────────
struct Vec2 {
    double x = 0, y = 0;

    Vec2() = default;
    Vec2(double x, double y) : x(x), y(y) {}

    Vec2  operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2  operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2  operator*(double s)      const { return {x*s, y*s}; }
    Vec2  operator-()              const { return {-x, -y}; }

    double dot(const Vec2& o)  const { return x*o.x + y*o.y; }
    double cross(const Vec2& o)const { return x*o.y - y*o.x; }
    double norm2()             const { return x*x + y*y; }
    double norm()              const { return std::sqrt(norm2()); }
    Vec2   normalized()        const { double n = norm(); return n>1e-12 ? Vec2{x/n, y/n} : Vec2{}; }

    // Triple product: (A×B)×C = B(A·C) - A(B·C)
    static Vec2 tripleProduct(const Vec2& a, const Vec2& b, const Vec2& c) {
        double ac = a.dot(c), bc = b.dot(c);
        return {b.x*ac - a.x*bc, b.y*ac - a.y*bc};
    }
};

// ─── Axis-Aligned Bounding Box ───────────────────────────────────────────────
struct AABB {
    double minX, minY, maxX, maxY;

    bool overlaps(const AABB& o) const {
        return minX <= o.maxX && maxX >= o.minX &&
               minY <= o.maxY && maxY >= o.minY;
    }

    // Expand AABB to include a point
    void include(const Vec2& p) {
        minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
    }

    static AABB fromPoints(const std::vector<Vec2>& pts) {
        AABB b{1e18, 1e18, -1e18, -1e18};
        for (auto& p : pts) b.include(p);
        return b;
    }
};
