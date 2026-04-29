#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/WilliamsonFListBuilder.hpp"
#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"
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

StrongPlanarityFailure computeFailure(const PreparedPalmTree& prepared) {
    StrongPlanarityTester tester(prepared, prepared.number);
    std::vector<Side> alpha;

    bool planar = tester.run(prepared.rootTreeDart, alpha);

    assert(!planar);

    return tester.failure();
}

WilliamsonContext buildContext(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const StrongPlanarityFailure& failure
) {
    WilliamsonContextBuilder builder;
    WilliamsonContext context =
        builder.build(prepared, pathTree, metadata, failure);

    assert(context.valid);

    return context;
}

WilliamsonSegmentList buildSegmentList(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const WilliamsonContext& context
) {
    WilliamsonSegmentListBuilder builder;
    WilliamsonSegmentList segmentList =
        builder.build(prepared, pathTree, context);

    assert(segmentList.valid);

    return segmentList;
}

void assertFListConsistent(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const WilliamsonSegmentList& segmentList,
    const WilliamsonContext& context,
    const WilliamsonFList& fList
) {
    assert(fList.valid);
    assert(fList.segmentNodes == segmentList.segmentNodes);
    assert(fList.positionByNode.size() == pathTree.nodes.size());
    assert(fList.headsByNode.size() == pathTree.nodes.size());
    assert(fList.fxListByVertex.size() == static_cast<std::size_t>(prepared.n));

    assert(fList.fNode == context.fNode);
    assert(fList.fPosition != -1);
    assert(fList.segmentNodes[fList.fPosition] == context.fNode);

    for (int nodeId : fList.segmentNodes) {
        assert(nodeId >= 0);
        assert(nodeId < static_cast<int>(pathTree.nodes.size()));
        assert(fList.positionByNode[nodeId] != -1);
    }

    for (int vertex = 0; vertex < prepared.n; ++vertex) {
        for (int nodeId : fList.fxListByVertex[vertex]) {
            assert(nodeId >= 0);
            assert(nodeId < static_cast<int>(pathTree.nodes.size()));
            assert(fList.positionByNode[nodeId] != -1);
        }
    }
}

} // namespace

HT_TEST(WilliamsonFListBuilderBuildsFListForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentList segmentList =
        buildSegmentList(prepared, pathTree, context);

    WilliamsonFListBuilder builder;
    WilliamsonFList fList =
        builder.buildFromSegmentList(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context.fNode
        );

    assertFListConsistent(
        prepared,
        pathTree,
        segmentList,
        context,
        fList
    );
}

HT_TEST(WilliamsonFListBuilderBuildsFxListsForSubdividedK5) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentList segmentList =
        buildSegmentList(prepared, pathTree, context);

    WilliamsonFListBuilder builder;
    WilliamsonFList fList =
        builder.buildFromSegmentList(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context.fNode
        );

    assertFListConsistent(
        prepared,
        pathTree,
        segmentList,
        context,
        fList
    );

    bool foundNonEmptyFxList = false;

    for (const std::vector<int>& list : fList.fxListByVertex) {
        if (!list.empty()) {
            foundNonEmptyFxList = true;
            break;
        }
    }

    assert(foundNonEmptyFxList);
}

HT_TEST(WilliamsonFListBuilderRejectsMissingFNode) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentList segmentList =
        buildSegmentList(prepared, pathTree, context);

    WilliamsonFListBuilder builder;
    WilliamsonFList fList =
        builder.buildFromSegmentList(
            prepared,
            pathTree,
            metadata,
            segmentList,
            -1
        );

    assert(!fList.valid);
}