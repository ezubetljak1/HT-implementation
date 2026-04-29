#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
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

} // namespace

HT_TEST(WilliamsonContextBuilderMapsK33FailureToFAB) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContextBuilder builder;
    WilliamsonContext context =
        builder.build(prepared, pathTree, metadata, failure);

    assert(context.valid);
    assert(context.fNode != -1);
    assert(context.aNode != -1);
    assert(context.bNode != -1);

    assert(context.fDart != -1);

    if (!failure.cycleRootEmanatingDarts.empty()) {
        assert(context.fDart == failure.cycleRootEmanatingDarts.front());
    }
    assert(context.aDart != -1);
    assert(context.bDart != -1);

    assert(context.aLinkedToF);
    assert(context.bLinkedToF);

    assert(!context.message.empty());
}

HT_TEST(WilliamsonContextBuilderMapsSubdividedK5FailureToFAB) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContextBuilder builder;
    WilliamsonContext context =
        builder.build(prepared, pathTree, metadata, failure);

    assert(context.valid);
    assert(context.fNode != -1);
    assert(context.aNode != -1);
    assert(context.bNode != -1);

    assert(context.fDart != -1);
    assert(context.aLinkedToF);
    assert(context.bLinkedToF);
}

HT_TEST(WilliamsonContextBuilderRejectsEmptyFailure) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);

    StrongPlanarityFailure emptyFailure;

    WilliamsonContextBuilder builder;
    WilliamsonContext context =
        builder.build(prepared, pathTree, metadata, emptyFailure);

    assert(!context.valid);
    assert(context.fNode == -1);
    assert(context.aNode == -1);
    assert(context.bNode == -1);
    assert(!context.message.empty());
}