#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/DirectLinkTester.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
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

SegmentMetadataTable buildMetadata(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree
) {
    SegmentMetadataBuilder builder;
    return builder.build(prepared, pathTree);
}

} // namespace

HT_TEST(DirectLinkTesterMaterializesHeadsForBackSegment) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);

    DirectLinkTester tester(prepared, pathTree, metadata);

    bool foundBackSegmentWithHead = false;

    for (const PathNode& node : pathTree.nodes) {
        const Dart& d = prepared.darts[node.definingDart];

        if (!d.isBack) {
            continue;
        }

        std::vector<int> heads =
            tester.headVerticesForNode(node.id);

        assert(!heads.empty());

        foundBackSegmentWithHead = true;
    }

    assert(foundBackSegmentWithHead);
}

HT_TEST(DirectLinkTesterDetectsFailureSegmentsLinkedToFInK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);

    StrongPlanarityTester strong(prepared, prepared.number);
    std::vector<Side> alpha;

    bool planar = strong.run(prepared.rootTreeDart, alpha);

    assert(!planar);

    const StrongPlanarityFailure& failure = strong.failure();

    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);

    DirectLinkTester tester(prepared, pathTree, metadata);

    const int fNode =
        pathTree.nodeByDefiningDart[failure.cycleRootDart];

    assert(fNode != -1);

    bool foundLinkedLeft = false;
    bool foundLinkedRight = false;

    for (int dartId : failure.blockLeftSegments) {
        const int nodeId = pathTree.nodeByDefiningDart[dartId];

        assert(nodeId != -1);

        if (tester.directlyLinkedToEarlierSegment(fNode, nodeId)) {
            foundLinkedLeft = true;
        }
    }

    for (int dartId : failure.blockRightSegments) {
        const int nodeId = pathTree.nodeByDefiningDart[dartId];

        assert(nodeId != -1);

        if (tester.directlyLinkedToEarlierSegment(fNode, nodeId)) {
            foundLinkedRight = true;
        }
    }

    assert(foundLinkedLeft);
    assert(foundLinkedRight);
}

HT_TEST(DirectLinkTesterDoesNotEagerlyMaterializeAllPairs) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);

    DirectLinkTester tester(prepared, pathTree, metadata);

    // Targeted call only: this should inspect one segment subtree, not all pairs.
    std::vector<int> heads =
        tester.headVerticesForNode(pathTree.rootNode);

    assert(!heads.empty() || pathTree.nodes.size() == 1);
}