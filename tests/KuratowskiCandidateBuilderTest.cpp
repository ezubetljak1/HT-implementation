#include "TestSupport.hpp"

#include <vector>

#include "TestGraphs.hpp"
#include "ht/Graph.hpp"
#include "ht/PlanarityTester.hpp"

using namespace ht;

namespace {

Graph buildSubgraphFromOriginalEdgeIds(
    const Graph& originalGraph,
    const std::vector<int>& originalEdgeIds
) {
    std::vector<char> selected(
        static_cast<std::size_t>(originalGraph.edgeCount()),
        0
    );

    for (int originalEdgeId : originalEdgeIds) {
        assert(originalEdgeId >= 0);
        assert(originalEdgeId < originalGraph.edgeCount());

        selected[originalEdgeId] = 1;
    }

    Graph subgraph(originalGraph.vertexCount());

    for (const Edge& edge : originalGraph.edges()) {
        if (selected[edge.originalId]) {
            subgraph.addEdgeWithOriginalId(
                edge.u,
                edge.v,
                edge.originalId
            );
        }
    }

    return subgraph;
}

} // namespace

HT_TEST(KuratowskiCandidateForK33IsNonPlanar) {
    Graph g = ht::test::buildK33();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, false);

    assert(!result.planar);
    assert(!result.certificate.originalEdgeIds.empty());

    Graph candidate =
        buildSubgraphFromOriginalEdgeIds(
            g,
            result.certificate.originalEdgeIds
        );

    PlanarityResult candidateResult = tester.test(candidate, false);

    assert(!candidateResult.planar);
}

HT_TEST(KuratowskiCandidateForSubdividedK5HasValidOriginalEdges) {
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