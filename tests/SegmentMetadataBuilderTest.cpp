#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"

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

HT_TEST(SegmentMetadataBuilderBuildsMetadataForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable table = buildMetadata(prepared, pathTree);

    assert(table.segments.size() == pathTree.nodes.size());
    assert(table.segmentByNode.size() == pathTree.nodes.size());

    bool foundSegmentWithHead = false;

    for (const PathNode& node : pathTree.nodes) {
        const int segmentId = table.segmentByNode[node.id];

        assert(segmentId == node.id);

        const SegmentMetadata& metadata = table.segments[segmentId];

        assert(metadata.nodeId == node.id);
        assert(metadata.definingDart == node.definingDart);
        assert(metadata.tailVertex >= 0);
        assert(metadata.tailVertex < prepared.n);
        assert(metadata.tailDfsNumber > 0);

        assert(metadata.low1Dfs != -1);
        assert(metadata.low1Vertex != -1);

        if (metadata.hasHead()) {
            foundSegmentWithHead = true;
            assert(metadata.headLow1Dfs != -1);
            assert(metadata.headLow1Vertex != -1);
        }
    }

    assert(foundSegmentWithHead);
}

HT_TEST(SegmentMetadataBuilderAggregatesSubtreeHeadsForSubdividedK5) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable table = buildMetadata(prepared, pathTree);

    assert(table.segments.size() == pathTree.nodes.size());

    bool foundAggregatedSegment = false;

    for (const SegmentMetadata& metadata : table.segments) {
        assert(metadata.low1Dfs != -1);
        assert(metadata.low1Vertex != -1);

        if (metadata.headCount > 1) {
            foundAggregatedSegment = true;
            assert(metadata.headLow1Dfs != -1);
            assert(metadata.headLow1Vertex != -1);
        }

        if (metadata.hasLow2()) {
            assert(metadata.low2Vertex != -1);
            assert(metadata.low2Dfs >= metadata.low1Dfs);
        }
    }

    assert(foundAggregatedSegment);
}

HT_TEST(SegmentMetadataBuilderDoesNotMaterializeCyclesOrSegments) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);

    for (const PathNode& node : pathTree.nodes) {
        assert(node.cycleDarts.empty());
        assert(node.segmentDarts.empty());
    }

    SegmentMetadataTable table = buildMetadata(prepared, pathTree);

    assert(table.segments.size() == pathTree.nodes.size());
}