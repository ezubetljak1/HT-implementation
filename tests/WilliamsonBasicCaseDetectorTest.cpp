#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonBasicCaseDetector.hpp"
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

} // namespace

HT_TEST(WilliamsonBasicCaseDetectorFindsDirectCaseForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonBasicCaseDetector detector;
    WilliamsonBasicCase basicCase =
        detector.detect(prepared, pathTree, metadata, context);

    assert(basicCase.valid);
    assert(basicCase.type == WilliamsonBasicCaseType::DirectTriangleLinks);
    assert(basicCase.segmentPathNodes.size() == 2);

    assert(basicCase.segmentPathNodes[0] == context.bNode);
    assert(basicCase.segmentPathNodes[1] == context.aNode);

    assert(!basicCase.message.empty());
}

HT_TEST(WilliamsonBasicCaseDetectorHandlesSubdividedK5Context) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonBasicCaseDetector detector;
    WilliamsonBasicCase basicCase =
        detector.detect(prepared, pathTree, metadata, context);

    // This test intentionally does not require the direct basic case.
    // If basic case 1 is not found, the next Williamson step is SEGFO path construction.
    if (basicCase.valid) {
        assert(basicCase.type == WilliamsonBasicCaseType::DirectTriangleLinks);
        assert(basicCase.segmentPathNodes.size() == 2);
    } else {
        assert(basicCase.type == WilliamsonBasicCaseType::None);
    }

    assert(!basicCase.message.empty());
}

HT_TEST(WilliamsonBasicCaseDetectorRejectsInvalidContext) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);

    WilliamsonContext invalidContext;

    WilliamsonBasicCaseDetector detector;
    WilliamsonBasicCase basicCase =
        detector.detect(prepared, pathTree, metadata, invalidContext);

    assert(!basicCase.valid);
    assert(basicCase.type == WilliamsonBasicCaseType::None);
    assert(basicCase.segmentPathNodes.empty());
    assert(!basicCase.message.empty());
}