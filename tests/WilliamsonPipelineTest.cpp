#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/Graph.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/WilliamsonFListBuilder.hpp"
#include "ht/certificate/WilliamsonKernelBuilder.hpp"
#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"
#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

using namespace ht;

namespace {

struct WilliamsonPipelineOutput {
    PreparedPalmTree prepared;
    PathTree pathTree;
    SegmentMetadataTable metadata;
    StrongPlanarityFailure failure;
    WilliamsonContext context;
    WilliamsonSegmentList segmentList;
    WilliamsonFList fList;
    WilliamsonSegfoPath segfoPath;
    WilliamsonKernel kernel;
    KuratowskiSubdivisionVerification verification;
};

PreparedPalmTree prepareSingleComponent(const Graph& graph) {
    BiconnectedComponentsFinder finder;
    Components components = finder.find(graph);

    assert(components.size() == 1);

    ComponentPreprocessor preprocessor;
    PreprocessedComponent pc =
        preprocessor.preprocess(components[0]);

    PreparedPalmTreeBuilder builder;
    return builder.build(pc);
}

StrongPlanarityFailure computeFailure(const PreparedPalmTree& prepared) {
    StrongPlanarityTester tester(prepared, prepared.number);
    std::vector<Side> alpha;

    const bool planar =
        tester.run(prepared.rootTreeDart, alpha);

    assert(!planar);

    return tester.failure();
}

WilliamsonPipelineOutput runWilliamsonPipeline(const Graph& graph) {
    WilliamsonPipelineOutput output;

    output.prepared =
        prepareSingleComponent(graph);

    output.failure =
        computeFailure(output.prepared);

    PathTreeBuilder pathTreeBuilder;
    output.pathTree =
        pathTreeBuilder.build(output.prepared);

    SegmentMetadataBuilder metadataBuilder;
    output.metadata =
        metadataBuilder.build(
            output.prepared,
            output.pathTree
        );

    WilliamsonContextBuilder contextBuilder;
    output.context =
        contextBuilder.build(
            output.prepared,
            output.pathTree,
            output.metadata,
            output.failure
        );

    assert(output.context.valid);

    WilliamsonSegmentListBuilder segmentListBuilder;
    output.segmentList =
        segmentListBuilder.build(
            output.prepared,
            output.pathTree,
            output.context
        );

    assert(output.segmentList.valid);

    WilliamsonFListBuilder fListBuilder;
    output.fList =
        fListBuilder.buildFromSegmentList(
            output.prepared,
            output.pathTree,
            output.metadata,
            output.segmentList,
            output.context.fNode
        );

    assert(output.fList.valid);

    WilliamsonSegfoPathBuilder segfoPathBuilder;
    output.segfoPath =
        segfoPathBuilder.buildPathFromFList(
            output.prepared,
            output.pathTree,
            output.metadata,
            output.fList,
            output.context
        );

    assert(output.segfoPath.valid);
    assert(output.segfoPath.segmentPathNodes.size() >= 2);
    assert(output.segfoPath.segmentPathNodes.front() == output.context.bNode);
    assert(output.segfoPath.segmentPathNodes.back() == output.context.aNode);

    WilliamsonKernelBuilder kernelBuilder;
    output.kernel =
        kernelBuilder.buildKernelFromSegfoPath(
            output.prepared,
            output.pathTree,
            output.context,
            output.segfoPath
        );

    assert(output.kernel.valid);
    assert(!output.kernel.originalEdgeIds.empty());

    KuratowskiSubdivisionVerifier verifier;
    output.verification =
        verifier.verify(
            output.prepared,
            output.kernel.originalEdgeIds
        );

    assert(output.verification.valid);

    return output;
}

void assertNoDuplicateOriginalEdges(const std::vector<int>& edgeIds) {
    int maxId = -1;

    for (int edgeId : edgeIds) {
        assert(edgeId >= 0);

        if (edgeId > maxId) {
            maxId = edgeId;
        }
    }

    std::vector<char> seen(
        static_cast<std::size_t>(maxId + 1),
        0
    );

    for (int edgeId : edgeIds) {
        assert(!seen[edgeId]);
        seen[edgeId] = 1;
    }
}

} // namespace

HT_TEST(WilliamsonPipelineVerifiesK33Certificate) {
    Graph g = ht::test::buildK33();

    WilliamsonPipelineOutput output =
        runWilliamsonPipeline(g);

    assert(output.verification.type == KuratowskiType::K33Subdivision);
    assert(output.verification.originalEdgeIds.size() == 9);
    assertNoDuplicateOriginalEdges(output.verification.originalEdgeIds);
}

HT_TEST(WilliamsonPipelineVerifiesK5Certificate) {
    Graph g = ht::test::buildK5();

    WilliamsonPipelineOutput output =
        runWilliamsonPipeline(g);

    assert(output.verification.type == KuratowskiType::K5Subdivision);
    assert(output.verification.originalEdgeIds.size() == 10);
    assertNoDuplicateOriginalEdges(output.verification.originalEdgeIds);
}

HT_TEST(WilliamsonPipelineVerifiesSubdividedK5Certificate) {
    Graph g = ht::test::buildSubdividedK5();

    WilliamsonPipelineOutput output =
        runWilliamsonPipeline(g);

    assert(output.verification.type == KuratowskiType::K5Subdivision);
    assert(output.verification.originalEdgeIds.size() == 20);
    assertNoDuplicateOriginalEdges(output.verification.originalEdgeIds);
}