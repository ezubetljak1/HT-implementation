#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/PlanarityTester.hpp"

#include <vector>

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

HT_TEST(PlanarityPipelineBuildsGlobalEmbeddingForBridgeConnectedTriangles) {
    Graph g = ht::test::buildTwoTrianglesConnectedByBridge();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, true);

    assert(result.planar);
    assert(result.embedding.rotationOriginalEdgeIds.size() == 6);
    assert(result.embedding.rotationOriginalNeighbors.size() == 6);

    // Vertex 2 belongs to first triangle and bridge.
    assert(!result.embedding.rotationOriginalEdgeIds[2].empty());

    // Vertex 3 belongs to bridge and second triangle.
    assert(!result.embedding.rotationOriginalEdgeIds[3].empty());
}

HT_TEST(PlanarityPipelineGlobalEmbeddingCoversEveryOriginalEdgeTwice) {
    Graph g = ht::test::buildTwoTrianglesConnectedByBridge();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, true);

    assert(result.planar);
    assert(result.embedding.rotationOriginalEdgeIds.size() == 6);

    std::vector<int> occurrenceCount(g.edgeCount(), 0);

    for (const auto& rotation : result.embedding.rotationOriginalEdgeIds) {
        for (int originalEdgeId : rotation) {
            assert(originalEdgeId >= 0);
            assert(originalEdgeId < g.edgeCount());

            occurrenceCount[originalEdgeId]++;
        }
    }

    for (int count : occurrenceCount) {
        assert(count == 2);
    }
}

HT_TEST(PlanarityPipelineNonPlanarResultCarriesFailureWitnessEdgesForK33) {
    Graph g = ht::test::buildK33();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, false);

    assert(!result.planar);
    assert(!result.certificate.originalEdgeIds.empty());

    for (int originalEdgeId : result.certificate.originalEdgeIds) {
        assert(originalEdgeId >= 0);
        assert(originalEdgeId < g.edgeCount());
    }
}

HT_TEST(PlanarityPipelineNonPlanarResultCarriesFailureWitnessEdgesForSubdividedK5) {
    Graph g = ht::test::buildSubdividedK5();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, false);

    assert(!result.planar);
    assert(!result.certificate.originalEdgeIds.empty());

    for (int originalEdgeId : result.certificate.originalEdgeIds) {
        assert(originalEdgeId >= 0);
        assert(originalEdgeId < g.edgeCount());
    }
}