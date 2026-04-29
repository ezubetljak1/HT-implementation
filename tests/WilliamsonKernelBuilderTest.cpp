#include "TestSupport.hpp"

#include <vector>

#include "TestGraphs.hpp"
#include "ht/Graph.hpp"
#include "ht/PlanarityTester.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonBasicCaseDetector.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/WilliamsonKernelBuilder.hpp"
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

WilliamsonBasicCase detectBasicCase(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context
) {
    WilliamsonBasicCaseDetector detector;
    return detector.detect(prepared, pathTree, metadata, context);
}

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

HT_TEST(WilliamsonKernelBuilderBuildsNonPlanarDirectKernelForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonBasicCase basicCase =
        detectBasicCase(prepared, pathTree, metadata, context);

    assert(basicCase.valid);
    assert(basicCase.type == WilliamsonBasicCaseType::DirectTriangleLinks);

    WilliamsonKernelBuilder kernelBuilder;
    WilliamsonKernel kernel =
        kernelBuilder.buildDirectKernel(
            prepared,
            pathTree,
            basicCase
        );

    assert(kernel.valid);
    assert(!kernel.originalEdgeIds.empty());

    for (int originalEdgeId : kernel.originalEdgeIds) {
        assert(originalEdgeId >= 0);
        assert(originalEdgeId < g.edgeCount());
    }

    Graph kernelGraph =
        buildSubgraphFromOriginalEdgeIds(
            g,
            kernel.originalEdgeIds
        );

    PlanarityTester tester;
    PlanarityResult kernelResult =
        tester.test(kernelGraph, false);

    assert(!kernelResult.planar);
}

HT_TEST(WilliamsonKernelBuilderHandlesSubdividedK5WhenDirectCaseExists) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);
    SegmentMetadataTable metadata = buildMetadata(prepared, pathTree);
    StrongPlanarityFailure failure = computeFailure(prepared);

    WilliamsonContext context =
        buildContext(prepared, pathTree, metadata, failure);

    WilliamsonBasicCase basicCase =
        detectBasicCase(prepared, pathTree, metadata, context);

    if (!basicCase.valid) {
        return;
    }

    WilliamsonKernelBuilder kernelBuilder;
    WilliamsonKernel kernel =
        kernelBuilder.buildDirectKernel(
            prepared,
            pathTree,
            basicCase
        );

    assert(kernel.valid);
    assert(!kernel.originalEdgeIds.empty());

    for (int originalEdgeId : kernel.originalEdgeIds) {
        assert(originalEdgeId >= 0);
        assert(originalEdgeId < g.edgeCount());
    }
}

HT_TEST(WilliamsonKernelBuilderRejectsInvalidBasicCase) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);
    PathTree pathTree = buildPathTree(prepared);

    WilliamsonBasicCase invalidBasicCase;

    WilliamsonKernelBuilder kernelBuilder;
    WilliamsonKernel kernel =
        kernelBuilder.buildDirectKernel(
            prepared,
            pathTree,
            invalidBasicCase
        );

    assert(!kernel.valid);
    assert(kernel.originalEdgeIds.empty());
    assert(!kernel.message.empty());
}