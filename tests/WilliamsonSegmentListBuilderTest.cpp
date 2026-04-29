#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
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

void assertSegmentListContainsFAB(
    const WilliamsonSegmentList& segmentList,
    const WilliamsonContext& context
) {
    assert(segmentList.valid);
    assert(segmentList.baseNode == context.cycleNode);

    assert(segmentList.fPosition != -1);
    assert(segmentList.aPosition != -1);
    assert(segmentList.bPosition != -1);

    assert(segmentList.segmentNodes[segmentList.fPosition] == context.fNode);
    assert(segmentList.segmentNodes[segmentList.aPosition] == context.aNode);
    assert(segmentList.segmentNodes[segmentList.bPosition] == context.bNode);

    assert(!segmentList.message.empty());
}

} // namespace

HT_TEST(WilliamsonSegmentListBuilderBuildsSeglistForK33Context) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentListBuilder builder;
    WilliamsonSegmentList segmentList =
        builder.build(prepared, pathTree, context);

    assertSegmentListContainsFAB(segmentList, context);
}

HT_TEST(WilliamsonSegmentListBuilderBuildsSeglistForSubdividedK5Context) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonSegmentListBuilder builder;
    WilliamsonSegmentList segmentList =
        builder.build(prepared, pathTree, context);

    assertSegmentListContainsFAB(segmentList, context);
}

HT_TEST(WilliamsonSegmentListBuilderRejectsInvalidContext) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);

    WilliamsonContext invalidContext;

    WilliamsonSegmentListBuilder builder;
    WilliamsonSegmentList segmentList =
        builder.build(prepared, pathTree, invalidContext);

    assert(!segmentList.valid);
    assert(segmentList.baseNode == -1);
    assert(segmentList.segmentNodes.empty());
    assert(!segmentList.message.empty());
}