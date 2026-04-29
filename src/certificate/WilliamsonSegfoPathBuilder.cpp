#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"

#include <stdexcept>

namespace ht {

WilliamsonSegfoPath WilliamsonSegfoPathBuilder::buildPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonSegmentList& segmentList,
    const WilliamsonContext& context
) const {
    WilliamsonSegfoPath result;

    if (!context.valid) {
        result.message = "Cannot build SEGFO path from invalid Williamson context.";
        return result;
    }

    if (!segmentList.valid) {
        result.message = "Cannot build SEGFO path from invalid SEGLIST(e).";
        return result;
    }

    HeadCache headCache =
        buildHeadCache(
            prepared,
            pathTree,
            metadata,
            segmentList
        );

    // Direct SEGFO path:
    // B dl A.
    if (directlyLinkedEitherDirection(
            prepared,
            metadata,
            headCache,
            context.bNode,
            context.aNode
        )) {
        result.valid = true;
        result.segmentPathNodes.push_back(context.bNode);
        result.segmentPathNodes.push_back(context.aNode);
        result.message = "Built direct SEGFO path: B -> A.";
        return result;
    }

    std::vector<char> usedNode(
        static_cast<std::size_t>(pathTree.nodes.size()),
        0
    );

    int currentNode = context.bNode;

    result.segmentPathNodes.push_back(context.bNode);
    usedNode[context.bNode] = 1;

    // General targeted SEGFO path construction:
    // one linear pass over SEGLIST(e), extending the current path whenever
    // the next segment is directly linked to the current end.
    //
    // This avoids all-pairs SEGGR construction.
    for (int candidateNode : segmentList.segmentNodes) {
        if (candidateNode < 0 || candidateNode >= static_cast<int>(pathTree.nodes.size())) {
            result.message = "SEGLIST(e) contains invalid PathTree node id.";
            return result;
        }

        if (isContextNode(context, candidateNode)) {
            continue;
        }

        if (usedNode[candidateNode]) {
            continue;
        }

        const bool currentLinkedToCandidate =
            directlyLinkedEitherDirection(
                prepared,
                metadata,
                headCache,
                currentNode,
                candidateNode
            );

        if (!currentLinkedToCandidate) {
            continue;
        }

        result.segmentPathNodes.push_back(candidateNode);
        usedNode[candidateNode] = 1;
        currentNode = candidateNode;

        const bool currentLinkedToA =
            directlyLinkedEitherDirection(
                prepared,
                metadata,
                headCache,
                currentNode,
                context.aNode
            );

        if (currentLinkedToA) {
            result.segmentPathNodes.push_back(context.aNode);
            result.valid = true;
            result.message = "Built linear SEGFO path: B -> ... -> A.";
            return result;
        }
    }

    result.message =
        "No SEGFO path found by the current one-pass construction. "
        "The next Williamson step is FLIST/frontier construction for the fully general case.";

    return result;
}

WilliamsonSegfoPathBuilder::HeadCache
WilliamsonSegfoPathBuilder::buildHeadCache(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonSegmentList& segmentList
) {
    HeadCache cache;
    cache.headsByNode.resize(pathTree.nodes.size());

    DirectLinkTester directLinkTester(
        prepared,
        pathTree,
        metadata
    );

    for (int nodeId : segmentList.segmentNodes) {
        if (nodeId < 0 || nodeId >= static_cast<int>(pathTree.nodes.size())) {
            throw std::runtime_error("Invalid node id while building SEGFO head cache.");
        }

        cache.headsByNode[nodeId] =
            directLinkTester.headVerticesForNode(nodeId);
    }

    return cache;
}

bool WilliamsonSegfoPathBuilder::directlyLinkedEitherDirection(
    const PreparedPalmTree& prepared,
    const SegmentMetadataTable& metadata,
    const HeadCache& headCache,
    int firstNode,
    int secondNode
) {
    return directlyLinkedEarlierLater(
            prepared,
            metadata,
            headCache,
            firstNode,
            secondNode
        )
        || directlyLinkedEarlierLater(
            prepared,
            metadata,
            headCache,
            secondNode,
            firstNode
        );
}

bool WilliamsonSegfoPathBuilder::directlyLinkedEarlierLater(
    const PreparedPalmTree& prepared,
    const SegmentMetadataTable& metadata,
    const HeadCache& headCache,
    int earlierNode,
    int laterNode
) {
    const SegmentMetadata& earlier =
        metadataForNode(metadata, earlierNode);

    if (earlier.low1Dfs == -1 || earlier.tailDfsNumber == -1) {
        return false;
    }

    return hasHeadInOpenDfsInterval(
        prepared,
        headCache,
        laterNode,
        earlier.low1Dfs,
        earlier.tailDfsNumber
    );
}

bool WilliamsonSegfoPathBuilder::hasHeadInOpenDfsInterval(
    const PreparedPalmTree& prepared,
    const HeadCache& headCache,
    int nodeId,
    int lowExclusiveDfs,
    int highExclusiveDfs
) {
    if (nodeId < 0 || nodeId >= static_cast<int>(headCache.headsByNode.size())) {
        return false;
    }

    if (lowExclusiveDfs >= highExclusiveDfs) {
        return false;
    }

    const std::vector<int>& heads =
        headCache.headsByNode[nodeId];

    for (int vertex : heads) {
        if (vertex < 0 || vertex >= prepared.n) {
            continue;
        }

        const int dfs = prepared.number[vertex];

        if (dfs > lowExclusiveDfs && dfs < highExclusiveDfs) {
            return true;
        }
    }

    return false;
}

const SegmentMetadata& WilliamsonSegfoPathBuilder::metadataForNode(
    const SegmentMetadataTable& metadata,
    int nodeId
) {
    if (nodeId < 0 || nodeId >= static_cast<int>(metadata.segmentByNode.size())) {
        throw std::runtime_error("Invalid node id while reading SegmentMetadata.");
    }

    const int segmentId = metadata.segmentByNode[nodeId];

    if (segmentId < 0 || segmentId >= static_cast<int>(metadata.segments.size())) {
        throw std::runtime_error("Invalid segment id while reading SegmentMetadata.");
    }

    return metadata.segments[segmentId];
}

bool WilliamsonSegfoPathBuilder::isContextNode(
    const WilliamsonContext& context,
    int nodeId
) {
    return nodeId == context.fNode
        || nodeId == context.aNode
        || nodeId == context.bNode
        || nodeId == context.cycleNode;
}

} // namespace ht