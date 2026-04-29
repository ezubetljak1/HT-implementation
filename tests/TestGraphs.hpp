#pragma once

#include "ht/Graph.hpp"

namespace ht::test {

inline Graph buildEmptyGraph() {
    return Graph(0);
}

inline Graph buildSingleVertex() {
    return Graph(1);
}

inline Graph buildSingleEdge() {
    Graph g(2);
    g.addEdge(0, 1);
    return g;
}

inline Graph buildPathGraph() {
    Graph g(4);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    return g;
}

inline Graph buildCycleGraph() {
    Graph g(4);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 0);
    return g;
}

inline Graph buildTriangle() {
    Graph g(3);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    return g;
}

inline Graph buildK4() {
    Graph g(4);
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            g.addEdge(i, j);
        }
    }
    return g;
}

inline Graph buildK5() {
    Graph g(5);
    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            g.addEdge(i, j);
        }
    }
    return g;
}

inline Graph buildK33() {
    Graph g(6);

    for (int u = 0; u < 3; ++u) {
        for (int v = 3; v < 6; ++v) {
            g.addEdge(u, v);
        }
    }

    return g;
}

inline Graph buildTwoTrianglesConnectedByBridge() {
    Graph g(6);

    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);

    g.addEdge(2, 3);

    g.addEdge(3, 4);
    g.addEdge(4, 5);
    g.addEdge(5, 3);

    return g;
}

inline Graph buildWheelGraph5() {
    Graph g(6);

    // outer cycle 0-1-2-3-4-0
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 0);

    // center 5 connected to all outer vertices
    for (int v = 0; v < 5; ++v) {
        g.addEdge(5, v);
    }

    return g;
}

inline Graph buildSubdividedK5() {
    // K5 has 5 original branch vertices: 0..4.
    // Each K5 edge is subdivided once using vertices 5..14.
    // This graph is still non-planar, but it avoids the trivial density shortcut.
    Graph g(15);

    int subdivisionVertex = 5;

    for (int u = 0; u < 5; ++u) {
        for (int v = u + 1; v < 5; ++v) {
            int s = subdivisionVertex++;

            g.addEdge(u, s);
            g.addEdge(s, v);
        }
    }

    return g;
}

} // namespace ht::test
