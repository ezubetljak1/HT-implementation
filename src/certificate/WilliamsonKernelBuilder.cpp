#include "ht/certificate/WilliamsonKernelBuilder.hpp"

namespace ht {

WilliamsonKernel WilliamsonKernelBuilder::buildDirectKernel(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const WilliamsonBasicCase& basicCase
) const {
    if (!basicCase.valid) {
        WilliamsonKernel kernel;
        kernel.basicCase = basicCase;
        kernel.message = "Cannot build Williamson kernel from invalid basic case.";
        return kernel;
    }

    WilliamsonSegfoPath path;
    path.valid = true;
    path.segmentPathNodes = basicCase.segmentPathNodes;
    path.message = "SEGFO path induced by direct Williamson basic case.";

    WilliamsonKernel kernel =
        buildKernelFromSegfoPath(
            prepared,
            pathTree,
            basicCase.context,
            path
        );

    kernel.basicCase = basicCase;
    return kernel;
}

WilliamsonKernel WilliamsonKernelBuilder::buildKernelFromSegfoPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonKernel kernel;
    kernel.context = context;
    kernel.segfoPath = segfoPath;

    if (!context.valid) {
        kernel.message = "Cannot build Williamson kernel from invalid context.";
        return kernel;
    }

    if (!segfoPath.valid || segfoPath.segmentPathNodes.empty()) {
        kernel.message = "Cannot build Williamson kernel from invalid SEGFO path.";
        return kernel;
    }

    const int maxOriginalId = maxOriginalEdgeId(prepared);

    if (maxOriginalId < 0) {
        kernel.message = "Prepared graph contains no original edge IDs.";
        return kernel;
    }

    std::vector<char> seenEdge(static_cast<std::size_t>(maxOriginalId + 1), 0);
    std::vector<char> seenPathNode(static_cast<std::size_t>(pathTree.nodes.size()), 0);

    PathTreeQueries queries(prepared, pathTree);

    // Williamson kernel uses the base failure cycle CYCLE(e).
    addCycleForNode(
        prepared,
        queries,
        context.cycleNode,
        seenEdge,
        kernel.originalEdgeIds
    );

    // Add SEG(F).
    addSegmentSubtreeForNode(
        prepared,
        pathTree,
        context.fNode,
        seenPathNode,
        seenEdge,
        kernel.originalEdgeIds
    );

    // Add all segments on the SEGFO path B -> ... -> A.
    for (int nodeId : segfoPath.segmentPathNodes) {
        addSegmentSubtreeForNode(
            prepared,
            pathTree,
            nodeId,
            seenPathNode,
            seenEdge,
            kernel.originalEdgeIds
        );
    }

    kernel.valid = !kernel.originalEdgeIds.empty();

    if (kernel.valid) {
        kernel.message = "Built Williamson kernel from base cycle, F, and SEGFO path.";
    } else {
        kernel.message = "Williamson kernel construction produced no original edges.";
    }

    return kernel;
}

int WilliamsonKernelBuilder::maxOriginalEdgeId(
    const PreparedPalmTree& prepared
) {
    int maxId = -1;

    for (const Dart& dart : prepared.darts) {
        if (dart.originalEdgeId > maxId) {
            maxId = dart.originalEdgeId;
        }
    }

    return maxId;
}

void WilliamsonKernelBuilder::addUniqueOriginalEdgeFromDart(
    const PreparedPalmTree& prepared,
    int dartId,
    std::vector<char>& seenEdge,
    std::vector<int>& originalEdgeIds
) {
    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        return;
    }

    const int originalEdgeId = prepared.darts[dartId].originalEdgeId;

    if (originalEdgeId < 0 || originalEdgeId >= static_cast<int>(seenEdge.size())) {
        return;
    }

    if (seenEdge[originalEdgeId]) {
        return;
    }

    seenEdge[originalEdgeId] = 1;
    originalEdgeIds.push_back(originalEdgeId);
}

void WilliamsonKernelBuilder::addUniqueOriginalEdgesFromDarts(
    const PreparedPalmTree& prepared,
    const std::vector<int>& dartIds,
    std::vector<char>& seenEdge,
    std::vector<int>& originalEdgeIds
) {
    for (int dartId : dartIds) {
        addUniqueOriginalEdgeFromDart(
            prepared,
            dartId,
            seenEdge,
            originalEdgeIds
        );
    }
}

void WilliamsonKernelBuilder::addCycleForNode(
    const PreparedPalmTree& prepared,
    const PathTreeQueries& queries,
    int nodeId,
    std::vector<char>& seenEdge,
    std::vector<int>& originalEdgeIds
) {
    if (nodeId == -1) {
        return;
    }

    const std::vector<int> cycleDarts =
        queries.cycleDartsForNode(nodeId);

    addUniqueOriginalEdgesFromDarts(
        prepared,
        cycleDarts,
        seenEdge,
        originalEdgeIds
    );
}

void WilliamsonKernelBuilder::addSegmentSubtreeForNode(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    int nodeId,
    std::vector<char>& seenPathNode,
    std::vector<char>& seenEdge,
    std::vector<int>& originalEdgeIds
) {
    if (nodeId < 0 || nodeId >= static_cast<int>(pathTree.nodes.size())) {
        return;
    }

    const PathNode& segmentRoot = pathTree.nodes[nodeId];

    if (segmentRoot.preorder < 0 || segmentRoot.subtreeEnd < segmentRoot.preorder) {
        return;
    }

    if (segmentRoot.subtreeEnd > static_cast<int>(pathTree.preorderNodes.size())) {
        return;
    }

    for (int i = segmentRoot.preorder; i < segmentRoot.subtreeEnd; ++i) {
        const int currentNodeId = pathTree.preorderNodes[i];

        if (currentNodeId < 0 || currentNodeId >= static_cast<int>(pathTree.nodes.size())) {
            continue;
        }

        if (seenPathNode[currentNodeId]) {
            continue;
        }

        seenPathNode[currentNodeId] = 1;

        const PathNode& currentNode = pathTree.nodes[currentNodeId];

        addUniqueOriginalEdgesFromDarts(
            prepared,
            currentNode.pathDarts,
            seenEdge,
            originalEdgeIds
        );
    }
}

} // namespace ht