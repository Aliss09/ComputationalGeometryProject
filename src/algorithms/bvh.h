#pragma once
#include "math2d.h"
#include "object.h"
#include <vector>
#include <algorithm>

// ─── BVH Node ─────────────────────────────────────────────────────────────────
struct BVHNode {
    AABB box;
    int  left  = -1;   // child index (-1 = leaf)
    int  right = -1;
    int  objId = -1;   // object index (only set for leaves)
};

// ─── BVH Tree ─────────────────────────────────────────────────────────────────
// Top-down BVH built from object AABBs.
// Build: O(N log N)   |   Query: O(log N + K) on average
class BVH {
public:
    std::vector<BVHNode> nodes;
    int rootIdx = -1;

    void build(std::vector<Object>& objs) {
        nodes.clear();
        nodes.reserve(objs.size() * 2);
        std::vector<int> ids(objs.size());
        for (int i = 0; i < (int)objs.size(); i++) ids[i] = i;
        rootIdx = buildRecursive(objs, ids, 0, ids.size());
    }

    // Find all candidate pairs that may collide (AABB overlap).
    // Output: pairs vector of (i, j) where i < j.
    void queryPairs(std::vector<std::pair<int,int>>& outPairs) const {
        outPairs.clear();
        if (rootIdx < 0) return;
        queryRecursive(rootIdx, rootIdx, outPairs, true);
    }

private:
    int buildRecursive(std::vector<Object>& objs,
                        std::vector<int>& ids,
                        int begin, int end) {
        int nodeIdx = nodes.size();
        nodes.push_back(BVHNode{});

        // Compute combined AABB of all objects in this range
        AABB box = objs[ids[begin]].aabb();
        for (int i = begin+1; i < end; i++) {
            AABB b = objs[ids[i]].aabb();
            box.minX = std::min(box.minX, b.minX);
            box.minY = std::min(box.minY, b.minY);
            box.maxX = std::max(box.maxX, b.maxX);
            box.maxY = std::max(box.maxY, b.maxY);
        }
        nodes[nodeIdx].box = box;

        // Leaf case
        if (end - begin <= 1) {
            nodes[nodeIdx].objId = ids[begin];
            return nodeIdx;
        }

        // Choose split axis: longest dimension
        double dx = box.maxX - box.minX;
        double dy = box.maxY - box.minY;
        int axis = (dx > dy) ? 0 : 1;

        // Sort by center on chosen axis
        std::sort(ids.begin()+begin, ids.begin()+end,
            [&objs, axis](int a, int b) {
                AABB ba = objs[a].aabb(), bb = objs[b].aabb();
                double ca = (axis==0) ? (ba.minX+ba.maxX)*0.5 : (ba.minY+ba.maxY)*0.5;
                double cb = (axis==0) ? (bb.minX+bb.maxX)*0.5 : (bb.minY+bb.maxY)*0.5;
                return ca < cb;
            });

        int mid = (begin + end) / 2;
        int leftChild  = buildRecursive(objs, ids, begin, mid);
        int rightChild = buildRecursive(objs, ids, mid,   end);

        nodes[nodeIdx].left  = leftChild;
        nodes[nodeIdx].right = rightChild;
        return nodeIdx;
    }

    // Self-overlap query: find all pairs in tree whose AABBs overlap.
    void queryRecursive(int a, int b,
                        std::vector<std::pair<int,int>>& out,
                        bool sameNode) const {
        const BVHNode& na = nodes[a];
        const BVHNode& nb = nodes[b];

        if (!sameNode && !na.box.overlaps(nb.box)) return;

        bool aLeaf = (na.objId >= 0);
        bool bLeaf = (nb.objId >= 0);

        if (aLeaf && bLeaf) {
            if (a != b) {
                int i = std::min(na.objId, nb.objId);
                int j = std::max(na.objId, nb.objId);
                out.push_back({i, j});
            }
            return;
        }

        if (sameNode) {
            // Self-overlap of one node: descend both children + cross-pair
            queryRecursive(na.left,  na.left,  out, true);
            queryRecursive(na.right, na.right, out, true);
            queryRecursive(na.left,  na.right, out, false);
        } else {
            // Cross-overlap of two different nodes
            if (aLeaf) {
                queryRecursive(a, nb.left,  out, false);
                queryRecursive(a, nb.right, out, false);
            } else if (bLeaf) {
                queryRecursive(na.left,  b, out, false);
                queryRecursive(na.right, b, out, false);
            } else {
                queryRecursive(na.left,  nb.left,  out, false);
                queryRecursive(na.left,  nb.right, out, false);
                queryRecursive(na.right, nb.left,  out, false);
                queryRecursive(na.right, nb.right, out, false);
            }
        }
    }
};
