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

HT_TEST(PlanarityPipelineRejectsK33ByStrongPlanarity) {
    Graph g = ht::test::buildK33();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, false);

    assert(!result.planar);
}

HT_TEST(PlanarityPipelineAcceptsK4) {
    Graph g = ht::test::buildK4();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, true);

    assert(result.planar);
}

HT_TEST(PlanarityPipelineAcceptsWheelGraph5) {
    Graph g = ht::test::buildWheelGraph5();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, true);

    assert(result.planar);
}

HT_TEST(PlanarityPipelineRejectsSubdividedK5) {
    Graph g = ht::test::buildSubdividedK5();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, false);

    assert(!result.planar);
}