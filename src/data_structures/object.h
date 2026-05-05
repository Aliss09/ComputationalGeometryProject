#pragma once
#include "math2d.h"
#include "convex_hull.h"
#include <vector>
#include <cmath>

// ─── Object ───────────────────────────────────────────────────────────────────
struct Object {
    Vec2 pos;       // center position
    Vec2 vel;       // velocity (pixels/sec)
    double angle = 0;
    double angVel = 0;

    // Local-space polygon (original shape, non-convex OK)
    std::vector<Vec2> localShape;

    // Precomputed convex decomposition in local space
    std::vector<std::vector<Vec2>> localPieces;

    bool colliding = false; // set each frame by collision system

    // Build a star shape: outerR = outer radius, innerR = inner radius, n = points
    static Object makeStar(Vec2 pos, Vec2 vel, double outerR, double innerR, int n) {
        Object obj;
        obj.pos = pos; obj.vel = vel;
        for (int i = 0; i < 2*n; i++) {
            double a = i * M_PI / n - M_PI/2;
            double r = (i%2 == 0) ? outerR : innerR;
            obj.localShape.push_back({r*std::cos(a), r*std::sin(a)});
        }
        obj.buildPieces();
        return obj;
    }

    // Build an L-shape
    static Object makeL(Vec2 pos, Vec2 vel, double s) {
        Object obj;
        obj.pos = pos; obj.vel = vel;
        obj.localShape = {
            {0,0},{s,0},{s,s*0.4},{s*0.4,s*0.4},{s*0.4,s},{0,s}
        };
        obj.buildPieces();
        return obj;
    }

    // Build a box (convex baseline test)
    static Object makeBox(Vec2 pos, Vec2 vel, double w, double h) {
        Object obj;
        obj.pos = pos; obj.vel = vel;
        obj.localShape = {{-w/2,-h/2},{w/2,-h/2},{w/2,h/2},{-w/2,h/2}};
        obj.buildPieces();
        return obj;
    }

    
    // Decompose shape into convex pieces (call after setting localShape)
    
    static Object makeCrescent(Vec2 pos, Vec2 vel, double r) {
    Object obj;
    obj.pos = pos; obj.vel = vel;
    int n = 12;
    for (int i = 0; i <= n; i++) {
        double a = M_PI * i / n;
        obj.localShape.push_back({r * std::cos(a), r * std::sin(a)});
    }
    for (int i = n; i >= 0; i--) {
        double a = M_PI * i / n;
        obj.localShape.push_back({
            r*0.6*std::cos(a) + r*0.3,
            r*0.6*std::sin(a)
        });
    }
    obj.buildPieces();
    return obj;
    
}
    void buildPieces() {
        localPieces = convexDecompose(localShape);
    }

    // Get world-space pieces (apply position + rotation)
    std::vector<std::vector<Vec2>> worldPieces() const {
        std::vector<std::vector<Vec2>> result;
        double c = std::cos(angle), s = std::sin(angle);
        for (auto& piece : localPieces) {
            std::vector<Vec2> wp;
            for (auto& p : piece) {
                double rx = p.x*c - p.y*s;
                double ry = p.x*s + p.y*c;
                wp.push_back({pos.x+rx, pos.y+ry});
            }
            result.push_back(wp);
        }
        return result;
    }

    // Get world-space polygon (for rendering)
    std::vector<Vec2> worldShape() const {
        std::vector<Vec2> ws;
        double c = std::cos(angle), s = std::sin(angle);
        for (auto& p : localShape) {
            double rx = p.x*c - p.y*s;
            double ry = p.x*s + p.y*c;
            ws.push_back({pos.x+rx, pos.y+ry});
        }
        return ws;
    }

    AABB aabb() const {
        return AABB::fromPoints(worldShape());
    }

    void update(double dt, int screenW, int screenH) {
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
        angle += angVel * dt;

        // Bounce off walls
        auto bb = aabb();
        if (bb.minX < 0)       { pos.x -= bb.minX;        vel.x =  std::abs(vel.x); }
        if (bb.maxX > screenW) { pos.x -= (bb.maxX-screenW); vel.x = -std::abs(vel.x); }
        if (bb.minY < 0)       { pos.y -= bb.minY;        vel.y =  std::abs(vel.y); }
        if (bb.maxY > screenH) { pos.y -= (bb.maxY-screenH); vel.y = -std::abs(vel.y); }
    }
};
