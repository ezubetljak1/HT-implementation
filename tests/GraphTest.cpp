#include "TestSupport.hpp"

#include "TestGraphs.hpp"

using namespace ht;

HT_TEST(GraphStoresStableEdgeIds) {
    Graph g(3);

    int e0 = g.addEdge(0, 1);
    int e1 = g.addEdge(1, 2);

    assert(e0 == 0);
    assert(e1 == 1);

    assert(g.vertexCount() == 3);
    assert(g.edgeCount() == 2);

    assert(g.edge(0).u == 0);
    assert(g.edge(0).v == 1);
    assert(g.edge(0).originalId == 0);

    assert(g.edge(1).u == 1);
    assert(g.edge(1).v == 2);
    assert(g.edge(1).originalId == 1);
}
