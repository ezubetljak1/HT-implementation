#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/PlanarityTester.hpp"

using namespace ht;

HT_TEST(PlanarityPipelineAcceptsTriangle) {
    Graph g = ht::test::buildTriangle();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, true);

    assert(result.planar);
}

HT_TEST(PlanarityPipelineRejectsK5ByDensity) {
    Graph g = ht::test::buildK5();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, false);

    assert(!result.planar);
}
