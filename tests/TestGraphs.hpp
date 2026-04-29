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

inline Graph buildSubdividedK33() {
    Graph g(15);

    int subdivisionVertex = 6;

    for (int u = 0; u < 3; ++u) {
        for (int v = 3; v < 6; ++v) {
            int s = subdivisionVertex++;

            g.addEdge(u, s);
            g.addEdge(s, v);
        }
    }

    return g;
}

inline Graph buildStarGraph5() {
    Graph g(6);
    for (int v = 1; v < 6; ++v) {
        g.addEdge(0, v);
    }
    return g;
}

inline Graph buildButterflyGraph() {
    Graph g(5);

    // triangle 0-1-2-0
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);

    // triangle 0-3-4-0
    g.addEdge(0, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 0);

    return g;
}

inline Graph buildFigureEightGraph() {
    Graph g(5);

    // cycle 0-1-2-0
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);

    // cycle 0-3-4-0
    g.addEdge(0, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 0);

    return g;
}

inline Graph buildLadderGraph3() {
    Graph g(6);

    // left rail
    g.addEdge(0, 1);
    g.addEdge(1, 2);

    // right rail
    g.addEdge(3, 4);
    g.addEdge(4, 5);

    // rungs
    g.addEdge(0, 3);
    g.addEdge(1, 4);
    g.addEdge(2, 5);

    return g;
}

inline Graph buildTriangularPrism() {
    Graph g(6);

    // first triangle
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);

    // second triangle
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    g.addEdge(5, 3);

    // matching edges
    g.addEdge(0, 3);
    g.addEdge(1, 4);
    g.addEdge(2, 5);

    return g;
}

inline Graph buildCubeGraph() {
    Graph g(8);

    // front square
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 0);

    // back square
    g.addEdge(4, 5);
    g.addEdge(5, 6);
    g.addEdge(6, 7);
    g.addEdge(7, 4);

    // connecting edges
    g.addEdge(0, 4);
    g.addEdge(1, 5);
    g.addEdge(2, 6);
    g.addEdge(3, 7);

    return g;
}

inline Graph buildDisconnectedPlanarGraph() {
    Graph g(6);

    // triangle
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);

    // path
    g.addEdge(3, 4);
    g.addEdge(4, 5);

    return g;
}

inline Graph buildDisconnectedNonPlanarGraph() {
    Graph g(9);

    // triangle component
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);

    // K3,3 component on vertices 3..8
    for (int u = 3; u <= 5; ++u) {
        for (int v = 6; v <= 8; ++v) {
            g.addEdge(u, v);
        }
    }

    return g;
}

inline Graph buildK5MinusOneEdge() {
    Graph g(5);

    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            if (i == 0 && j == 1) {
                continue;
            }
            g.addEdge(i, j);
        }
    }

    return g;
}

inline Graph buildK33MinusOneEdge() {
    Graph g(6);

    for (int u = 0; u < 3; ++u) {
        for (int v = 3; v < 6; ++v) {
            if (u == 0 && v == 3) {
                continue;
            }
            g.addEdge(u, v);
        }
    }

    return g;
}

inline Graph buildPartiallySubdividedK5() {
    Graph g(6);

    // K5 on 0..4 except edge (0,1) is subdivided through 5
    g.addEdge(0, 5);
    g.addEdge(5, 1);

    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            if (i == 0 && j == 1) {
                continue;
            }
            g.addEdge(i, j);
        }
    }

    return g;
}

inline Graph buildPartiallySubdividedK33() {
    Graph g(7);

    // K3,3 on left {0,1,2}, right {3,4,5}
    // edge 0-3 is subdivided through 6
    g.addEdge(0, 6);
    g.addEdge(6, 3);

    for (int u = 0; u < 3; ++u) {
        for (int v = 3; v < 6; ++v) {
            if (u == 0 && v == 3) {
                continue;
            }
            g.addEdge(u, v);
        }
    }

    return g;
}

inline Graph buildPetersenGraph() {
    Graph g(10);

    // Outer 5-cycle.
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 0);

    // Inner star/pentagram cycle.
    g.addEdge(5, 7);
    g.addEdge(7, 9);
    g.addEdge(9, 6);
    g.addEdge(6, 8);
    g.addEdge(8, 5);

    // Spokes.
    g.addEdge(0, 5);
    g.addEdge(1, 6);
    g.addEdge(2, 7);
    g.addEdge(3, 8);
    g.addEdge(4, 9);

    return g;
}

inline Graph buildOctahedralGraph() {
    Graph g(6);

    // Octahedral graph = K6 minus 3 disjoint opposite pairs.
    // Opposite pairs: (0,1), (2,3), (4,5).
    for (int i = 0; i < 6; ++i) {
        for (int j = i + 1; j < 6; ++j) {
            const bool opposite =
                (i == 0 && j == 1)
                || (i == 2 && j == 3)
                || (i == 4 && j == 5);

            if (!opposite) {
                g.addEdge(i, j);
            }
        }
    }

    return g;
}

inline Graph buildDMRijeseni14a() {
    Graph g(6);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(0, 4);
    g.addEdge(0, 5);
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);
    g.addEdge(4, 5);

    return g;
}

inline Graph buildDMRijeseni14b() {
    Graph g(7);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(0, 5);
    g.addEdge(0, 6);
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 5);
    g.addEdge(2, 6);
    g.addEdge(3, 4);
    g.addEdge(3, 6);
    g.addEdge(4, 5);
    g.addEdge(4, 6);

    return g;
}

inline Graph buildDMRijeseni15() {
    Graph g(7);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(0, 3);
    g.addEdge(0, 5);
    g.addEdge(1, 2);
    g.addEdge(1, 6);
    g.addEdge(2, 3);
    g.addEdge(2, 5);
    g.addEdge(3, 4);
    g.addEdge(3, 6);
    g.addEdge(4, 5);
    g.addEdge(4, 6);

    return g;
}

inline Graph buildDMzsr10() {
    Graph g(10);

    g.addEdge(0, 1);
    g.addEdge(0, 3);
    g.addEdge(0, 4);
    g.addEdge(0, 7);
    g.addEdge(0, 9);

    g.addEdge(1, 2);
    g.addEdge(1, 8);

    g.addEdge(2, 5);
    g.addEdge(2, 6);
    g.addEdge(2, 7);
    g.addEdge(2, 9);

    g.addEdge(3, 7);

    g.addEdge(4, 5);
    g.addEdge(4, 7);

    g.addEdge(5, 9);

    g.addEdge(6, 9);

    g.addEdge(7, 8);

    g.addEdge(8, 9);

    return g;
}

inline Graph buildDMzsr14() {
    Graph g(10);

    g.addEdge(0, 1);
    g.addEdge(0, 5);
    g.addEdge(0, 6);

    g.addEdge(1, 2);
    g.addEdge(1, 5);
    g.addEdge(1, 6);
    g.addEdge(1, 7);

    g.addEdge(2, 3);
    g.addEdge(2, 6);
    g.addEdge(2, 7);
    g.addEdge(2, 8);

    g.addEdge(3, 4);
    g.addEdge(3, 7);
    g.addEdge(3, 8);
    g.addEdge(3, 9);

    g.addEdge(4, 8);
    g.addEdge(4, 9);

    g.addEdge(5, 6);

    g.addEdge(6, 7);

    g.addEdge(7, 8);

    g.addEdge(8, 9);

    return g;
}

} // namespace ht::test
