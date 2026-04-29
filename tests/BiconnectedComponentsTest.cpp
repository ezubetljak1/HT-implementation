#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"

using namespace ht;

HT_TEST(BiconnectedComponentsFindsSingleCycleComponent) {
    Graph g = ht::test::buildCycleGraph();

    BiconnectedComponentsFinder finder;
    Components components = finder.find(g);

    assert(components.size() == 1);
    assert(components[0].size() == 4);
}

HT_TEST(BiconnectedComponentsSplitsBridgeConnectedTriangles) {
    Graph g = ht::test::buildTwoTrianglesConnectedByBridge();

    BiconnectedComponentsFinder finder;
    Components components = finder.find(g);

    assert(components.size() == 3);
}
