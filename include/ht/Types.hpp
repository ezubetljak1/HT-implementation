#pragma once

#include <vector>

namespace ht {

struct Edge {
    int id = -1;          // ID inside the graph that currently owns this edge.
    int originalId = -1;  // ID of the edge in the original input graph.
    int u = -1;
    int v = -1;

    Edge() = default;

    Edge(int id, int originalId, int u, int v)
        : id(id), originalId(originalId), u(u), v(v) {}
};

struct AdjacentEdge {
    int to = -1;
    int edgeId = -1;

    AdjacentEdge() = default;

    AdjacentEdge(int to, int edgeId)
        : to(to), edgeId(edgeId) {}
};

using Component = std::vector<Edge>;
using Components = std::vector<Component>;

enum class ArcType {
    TreeArc,
    Frond
};

struct Arc {
    int edgeId = -1;          // local edge id
    int originalEdgeId = -1;  // original input edge id
    int from = -1;
    int to = -1;
    ArcType type = ArcType::TreeArc;
    int phi = -1;

    Arc() = default;

    Arc(int edgeId, int originalEdgeId, int from, int to, ArcType type)
        : edgeId(edgeId),
          originalEdgeId(originalEdgeId),
          from(from),
          to(to),
          type(type),
          phi(-1) {}
};

enum class Side {
    Left = 1,
    Right = 2
};

inline Side xorSide(Side a, Side b) {
    // Literature notation:
    // L xor L = L, R xor R = L, L xor R = R, R xor L = R
    return a == b ? Side::Left : Side::Right;
}

struct Dart {
    int id = -1;
    int edgeId = -1;          // local edge id
    int originalEdgeId = -1;  // original input edge id

    int from = -1;
    int to = -1;
    int rev = -1;

    bool isTree = false;
    bool isBack = false;

    // Filled only for active HT darts.
    int phi = -1;
};

struct LowpointInfo {
    int lowpt1 = -1;
    int lowpt2 = -1;

    int lowpt1Vertex = -1;
    int lowpt2Vertex = -1;

    int lowpt1EdgeId = -1;
    int lowpt2EdgeId = -1;

    int lowpt1OriginalEdgeId = -1;
    int lowpt2OriginalEdgeId = -1;
};

} // namespace ht
