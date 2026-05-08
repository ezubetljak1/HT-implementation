#include "TestSupport.hpp"

#include "ht/certificate/WilliamsonPathKernelBuilder.hpp"

#include <algorithm>
#include <vector>

using namespace ht;

namespace {

Dart makeTreeDart(
    int id,
    int originalEdgeId,
    int from,
    int to
) {
    Dart dart;
    dart.id = id;
    dart.edgeId = id;
    dart.originalEdgeId = originalEdgeId;
    dart.from = from;
    dart.to = to;
    dart.rev = -1;
    dart.isTree = true;
    dart.isBack = false;
    return dart;
}

Dart makeBackDart(
    int id,
    int originalEdgeId,
    int from,
    int to
) {
    Dart dart;
    dart.id = id;
    dart.edgeId = id;
    dart.originalEdgeId = originalEdgeId;
    dart.from = from;
    dart.to = to;
    dart.rev = -1;
    dart.isTree = false;
    dart.isBack = true;
    return dart;
}

PreparedPalmTree makePreparedPalmTreeForBasicCase1() {
    PreparedPalmTree prepared;
    prepared.n = 8;
    prepared.edgeCount = 14;

    prepared.number = {
        1, 2, 3, 4, 5, 6, 7, 8
    };

    prepared.parent = {
        -1, 0, 1, 2, 3, 4, 5, 6
    };

    prepared.darts.push_back(makeTreeDart(0, 0, 0, 1));
    prepared.darts.push_back(makeTreeDart(1, 1, 1, 2));
    prepared.darts.push_back(makeTreeDart(2, 2, 2, 3));
    prepared.darts.push_back(makeTreeDart(3, 3, 3, 4));
    prepared.darts.push_back(makeTreeDart(4, 4, 4, 5));
    prepared.darts.push_back(makeTreeDart(5, 5, 5, 6));
    prepared.darts.push_back(makeTreeDart(6, 6, 6, 7));

    // Base cycle back dart: 7 -> 0.
    prepared.darts.push_back(makeBackDart(7, 7, 7, 0));

    // A direct-link/reduced-segment witnesses.
    prepared.darts.push_back(makeBackDart(8, 8, 6, 2)); // A -> B
    prepared.darts.push_back(makeBackDart(9, 9, 7, 3)); // A -> F

    // B direct-link/reduced-segment witnesses.
    prepared.darts.push_back(makeBackDart(10, 10, 6, 3)); // B -> F
    prepared.darts.push_back(makeBackDart(11, 11, 5, 0));

    // F reduced-segment witnesses.
    prepared.darts.push_back(makeBackDart(12, 12, 6, 1));
    prepared.darts.push_back(makeBackDart(13, 13, 7, 2));

    prepared.orderedOut.assign(8, {});
    prepared.outAll.assign(8, {});
    prepared.alpha.assign(14, Side::Left);

    return prepared;
}

PathNode makeNode(
    int id,
    int definingDart,
    std::vector<int> pathDarts
) {
    PathNode node;
    node.id = id;
    node.definingDart = definingDart;
    node.kind = PathNodeKind::BackPath;
    node.parent = -1;
    node.children = {};
    node.pathDarts = std::move(pathDarts);
    node.preorder = id;
    node.subtreeEnd = id + 1;
    return node;
}

PathTree makePathTreeForBasicCase1() {
    PathTree pathTree;

    // node 0 = base CYCLE(e)
    // node 1 = F
    // node 2 = B
    // node 3 = A
    pathTree.nodes.push_back(makeNode(0, 7, {7}));
    pathTree.nodes.push_back(makeNode(1, 12, {12, 13}));
    pathTree.nodes.push_back(makeNode(2, 10, {10, 11}));
    pathTree.nodes.push_back(makeNode(3, 8, {8, 9}));

    pathTree.rootNode = 0;

    pathTree.preorderNodes = {
        0, 1, 2, 3
    };

    pathTree.nodeByDefiningDart.assign(14, -1);
    pathTree.nodeByDefiningDart[7] = 0;
    pathTree.nodeByDefiningDart[12] = 1;
    pathTree.nodeByDefiningDart[10] = 2;
    pathTree.nodeByDefiningDart[8] = 3;

    pathTree.treeDartFromParent = {
        -1, 0, 1, 2, 3, 4, 5, 6
    };

    return pathTree;
}

SegmentHeadWitness witness(
    int headVertex,
    int headDfs,
    int backDart,
    int backTailVertex
) {
    SegmentHeadWitness result;
    result.headVertex = headVertex;
    result.headDfs = headDfs;
    result.backDart = backDart;
    result.backTailVertex = backTailVertex;
    return result;
}

SegmentMetadata makeMetadata(
    int nodeId,
    int tailVertex,
    int tailDfs,
    int low1Dfs,
    int low1Vertex,
    SegmentHeadWitness low1Witness,
    int low2Dfs,
    int low2Vertex,
    SegmentHeadWitness low2Witness
) {
    SegmentMetadata metadata;
    metadata.nodeId = nodeId;
    metadata.definingDart = -1;
    metadata.parentNode = -1;
    metadata.tailVertex = tailVertex;
    metadata.tailDfsNumber = tailDfs;

    metadata.low1Dfs = low1Dfs;
    metadata.low1Vertex = low1Vertex;
    metadata.low1Witness = low1Witness;

    metadata.low2Dfs = low2Dfs;
    metadata.low2Vertex = low2Vertex;
    metadata.low2Witness = low2Witness;

    return metadata;
}

SegmentMetadataTable makeMetadataForBasicCase1() {
    SegmentMetadataTable table;

    table.segmentByNode = {
        0, 1, 2, 3
    };

    table.segments.resize(4);

    // Cycle node metadata is not used by the Basic Case 1 kernel.
    table.segments[0] =
        makeMetadata(
            0,
            7,
            8,
            -1,
            -1,
            SegmentHeadWitness{},
            -1,
            -1,
            SegmentHeadWitness{}
        );

    // F: OSPAN(F) is approximately (2, 5), so heads with dfs 3 or 4 link into F.
    table.segments[1] =
        makeMetadata(
            1,
            4,
            5,
            2,
            1,
            witness(1, 2, 12, 6),
            3,
            2,
            witness(2, 3, 13, 7)
        );

    // B: OSPAN(B) is approximately (1, 4), so A can link to B via head dfs 3.
    table.segments[2] =
        makeMetadata(
            2,
            3,
            4,
            1,
            0,
            witness(3, 4, 10, 6),
            3,
            2,
            witness(0, 1, 11, 5)
        );

    // A reduced segment witnesses.
    table.segments[3] =
        makeMetadata(
            3,
            5,
            6,
            3,
            2,
            witness(2, 3, 8, 6),
            4,
            3,
            witness(3, 4, 9, 7)
        );

    return table;
}

WilliamsonContext makeContextForBasicCase1() {
    WilliamsonContext context;
    context.valid = true;

    context.cycleNode = 0;
    context.cycleDart = 7;

    context.fNode = 1;
    context.bNode = 2;
    context.aNode = 3;

    context.fDart = 12;
    context.bDart = 10;
    context.aDart = 8;

    context.aLinkedToF = true;
    context.bLinkedToF = true;

    return context;
}

WilliamsonSegfoPath makeSegfoPathForBasicCase1() {
    WilliamsonSegfoPath path;
    path.valid = true;

    // Basic Case 1: B -> A.
    path.segmentPathNodes = {
        2, 3
    };

    return path;
}

bool containsEdge(
    const std::vector<int>& edges,
    int edgeId
) {
    return std::find(edges.begin(), edges.end(), edgeId) != edges.end();
}

} // namespace

HT_TEST(WilliamsonPathKernelBuilderBuildsBasicCase1FromWitnessPaths) {
    PreparedPalmTree prepared =
        makePreparedPalmTreeForBasicCase1();

    PathTree pathTree =
        makePathTreeForBasicCase1();

    SegmentMetadataTable metadata =
        makeMetadataForBasicCase1();

    WilliamsonContext context =
        makeContextForBasicCase1();

    WilliamsonSegfoPath segfoPath =
        makeSegfoPathForBasicCase1();

    WilliamsonPathKernelBuilder builder;

    WilliamsonKernel kernel =
        builder.buildBasicCase1(
            prepared,
            pathTree,
            metadata,
            context,
            segfoPath
        );

    assert(kernel.valid);
    assert(!kernel.originalEdgeIds.empty());

    // Base cycle edges.
    for (int edgeId = 0; edgeId <= 7; ++edgeId) {
        assert(containsEdge(kernel.originalEdgeIds, edgeId));
    }

    // Witness back edges from REDSEG/direct-link construction.
    assert(containsEdge(kernel.originalEdgeIds, 8));
    assert(containsEdge(kernel.originalEdgeIds, 9));
    assert(containsEdge(kernel.originalEdgeIds, 10));
    assert(containsEdge(kernel.originalEdgeIds, 11));
    assert(containsEdge(kernel.originalEdgeIds, 12));
    assert(containsEdge(kernel.originalEdgeIds, 13));

    // No duplicate original edge IDs.
    std::vector<int> sorted = kernel.originalEdgeIds;
    std::sort(sorted.begin(), sorted.end());

    const auto duplicate =
        std::adjacent_find(sorted.begin(), sorted.end());

    assert(duplicate == sorted.end());
}

HT_TEST(WilliamsonPathKernelBuilderRejectsNonBasicCase1SegfoPath) {
    PreparedPalmTree prepared =
        makePreparedPalmTreeForBasicCase1();

    PathTree pathTree =
        makePathTreeForBasicCase1();

    SegmentMetadataTable metadata =
        makeMetadataForBasicCase1();

    WilliamsonContext context =
        makeContextForBasicCase1();

    WilliamsonSegfoPath segfoPath;
    segfoPath.valid = true;

    // This represents B -> Y -> A, i.e. second Williamson basic case,
    // so Basic Case 1 builder must reject it.
    segfoPath.segmentPathNodes = {
        context.bNode,
        99,
        context.aNode
    };

    WilliamsonPathKernelBuilder builder;

    WilliamsonKernel kernel =
        builder.buildBasicCase1(
            prepared,
            pathTree,
            metadata,
            context,
            segfoPath
        );

    assert(!kernel.valid);
    assert(kernel.originalEdgeIds.empty());
}

HT_TEST(WilliamsonPathKernelBuilderReducesInternalYiLinkedToFIntoBasicCase1) {
    PreparedPalmTree prepared =
        makePreparedPalmTreeForBasicCase1();

    PathTree pathTree =
        makePathTreeForBasicCase1();

    SegmentMetadataTable metadata =
        makeMetadataForBasicCase1();

    WilliamsonContext context =
        makeContextForBasicCase1();

    // We fake a longer path:
    //
    //   B, Y1, A
    //
    // where Y1 is actually the old A node and is directly linked to F.
    // Because Y1 has odd Williamson index, normalization keeps B ... Y1,
    // which becomes B, A and therefore Basic Case 1.
    WilliamsonSegfoPath segfoPath;
    segfoPath.valid = true;
    segfoPath.segmentPathNodes = {
        context.bNode,
        context.aNode,
        context.aNode
    };

    WilliamsonPathKernelBuilder builder;

    WilliamsonKernel kernel =
        builder.build(
            prepared,
            pathTree,
            metadata,
            context,
            segfoPath
        );

    assert(kernel.valid);
    assert(!kernel.originalEdgeIds.empty());
}