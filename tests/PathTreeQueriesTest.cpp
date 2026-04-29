#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/PathTreeQueries.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

using namespace ht;

namespace {

PreparedPalmTree prepareSingleComponent(const Graph& graph) {
    BiconnectedComponentsFinder finder;
    Components components = finder.find(graph);

    assert(components.size() == 1);

    ComponentPreprocessor preprocessor;
    PreprocessedComponent pc = preprocessor.preprocess(components[0]);

    PreparedPalmTreeBuilder builder;
    return builder.build(pc);
}

PathTree buildPathTree(const PreparedPalmTree& prepared) {
    PathTreeBuilder builder;
    return builder.build(prepared);
}

} // namespace

HT_TEST(PathTreeQueriesMaterializesRootCycleForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);

    PathTreeQueries queries(prepared, pathTree);

    std::vector<int> cycle = queries.cycleDartsForNode(pathTree.rootNode);

    assert(!cycle.empty());

    bool hasBackDart = false;

    for (int dartId : cycle) {
        assert(dartId >= 0);
        assert(dartId < static_cast<int>(prepared.darts.size()));

        if (prepared.darts[dartId].isBack) {
            hasBackDart = true;
        }
    }

    assert(hasBackDart);
}

HT_TEST(PathTreeQueriesMapsStrongPlanarityFailureDartsForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);

    StrongPlanarityTester tester(prepared, prepared.number);
    std::vector<Side> alpha;

    bool planar = tester.run(prepared.rootTreeDart, alpha);

    assert(!planar);

    const StrongPlanarityFailure& failure = tester.failure();

    PathTree pathTree = buildPathTree(prepared);
    PathTreeQueries queries(prepared, pathTree);

    assert(queries.nodeForDefiningDart(failure.cycleRootDart) != -1);

    for (int dartId : failure.blockLeftSegments) {
        assert(queries.nodeForDefiningDart(dartId) != -1);
    }

    for (int dartId : failure.blockRightSegments) {
        assert(queries.nodeForDefiningDart(dartId) != -1);
    }
}

HT_TEST(PathTreeQueriesMaterializesSelectedSegmentByInterval) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);

    PathTreeQueries queries(prepared, pathTree);

    std::vector<int> segmentDarts =
        queries.segmentDefiningDartsForNode(pathTree.rootNode);

    assert(!segmentDarts.empty());
    assert(segmentDarts.size() == pathTree.nodes.size());

    for (int dartId : segmentDarts) {
        assert(dartId >= 0);
        assert(dartId < static_cast<int>(prepared.darts.size()));
    }
}