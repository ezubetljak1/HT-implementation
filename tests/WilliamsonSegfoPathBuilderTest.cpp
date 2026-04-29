#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"
#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"
#include "ht/certificate/WilliamsonFListBuilder.hpp"

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

void assertValidPathUsesContextEnds(
    const WilliamsonSegfoPath& path,
    const WilliamsonContext& context
) {
    assert(path.valid);
    assert(path.segmentPathNodes.size() >= 2);

    assert(path.segmentPathNodes.front() == context.bNode);
    assert(path.segmentPathNodes.back() == context.aNode);

    assert(!path.message.empty());
}

WilliamsonFList buildFList(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonSegmentList& segmentList,
    const WilliamsonContext& context
) {
    WilliamsonFListBuilder builder;
    WilliamsonFList fList =
        builder.buildFromSegmentList(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context.fNode
        );

    assert(fList.valid);

    return fList;
}

} // namespace

HT_TEST(WilliamsonSegfoPathBuilderFindsPathForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentList segmentList =
        buildSegmentList(prepared, pathTree, context);

    WilliamsonSegfoPathBuilder builder;
    WilliamsonSegfoPath path =
        builder.buildPath(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context
        );

    assertValidPathUsesContextEnds(path, context);
    assert(path.segmentPathNodes.size() >= 2);
}

HT_TEST(WilliamsonSegfoPathBuilderHandlesSubdividedK5Context) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentList segmentList =
        buildSegmentList(prepared, pathTree, context);

    WilliamsonSegfoPathBuilder builder;
    WilliamsonSegfoPath path =
        builder.buildPath(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context
        );

    assertValidPathUsesContextEnds(path, context);
}

HT_TEST(WilliamsonSegfoPathBuilderRejectsInvalidInputs) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);

    WilliamsonContext invalidContext;
    WilliamsonSegmentList invalidSegmentList;

    WilliamsonSegfoPathBuilder builder;
    WilliamsonSegfoPath path =
        builder.buildPath(
            prepared,
            pathTree,
            metadata,
            invalidSegmentList,
            invalidContext
        );

    assert(!path.valid);
    assert(path.segmentPathNodes.empty());
    assert(!path.message.empty());
}

HT_TEST(WilliamsonSegfoPathBuilderPathDoesNotRepeatNodesForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentList segmentList =
        buildSegmentList(prepared, pathTree, context);

    WilliamsonSegfoPathBuilder builder;
    WilliamsonSegfoPath path =
        builder.buildPath(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context
        );

    assertValidPathUsesContextEnds(path, context);

    std::vector<char> seen(
        static_cast<std::size_t>(pathTree.nodes.size()),
        0
    );

    for (int nodeId : path.segmentPathNodes) {
        assert(nodeId >= 0);
        assert(nodeId < static_cast<int>(pathTree.nodes.size()));
        assert(!seen[nodeId]);

        seen[nodeId] = 1;
    }
}

HT_TEST(WilliamsonSegfoPathBuilderFindsPathForK5) {
    Graph g = ht::test::buildK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentList segmentList =
        buildSegmentList(prepared, pathTree, context);

    WilliamsonSegfoPathBuilder builder;
    WilliamsonSegfoPath path =
        builder.buildPath(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context
        );

    assertValidPathUsesContextEnds(path, context);
}

HT_TEST(WilliamsonSegfoPathBuilderUsesExplicitFListForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentList segmentList =
        buildSegmentList(prepared, pathTree, context);

    WilliamsonFList fList =
        buildFList(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context
        );

    WilliamsonSegfoPathBuilder builder;
    WilliamsonSegfoPath path =
        builder.buildPathFromFList(
            prepared,
            pathTree,
            metadata,
            fList,
            context
        );

    assertValidPathUsesContextEnds(path, context);
}