#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"

#include "ht/certificate/WilliamsonFListBuilder.hpp"

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

    WilliamsonFListBuilder fListBuilder;
    WilliamsonFList fList =
        fListBuilder.buildFromSegmentList(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context.fNode
        );

    return buildPathFromFList(
        prepared,
        pathTree,
        metadata,
        fList,
        context
    );
}

WilliamsonSegfoPath WilliamsonSegfoPathBuilder::buildPathFromFList(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    const WilliamsonContext& context
) const {
    WilliamsonSegfoPath result;

    if (!context.valid) {
        result.message = "Cannot build SEGFO path from invalid Williamson context.";
        return result;
    }

    if (!fList.valid) {
        result.message = "Cannot build SEGFO path from invalid FLIST.";
        return result;
    }

    if (context.aNode < 0 || context.aNode >= static_cast<int>(pathTree.nodes.size())
        || context.bNode < 0 || context.bNode >= static_cast<int>(pathTree.nodes.size())
        || context.fNode < 0 || context.fNode >= static_cast<int>(pathTree.nodes.size())) {
        result.message = "Williamson context contains invalid node ids.";
        return result;
    }

    // Direct SEGFO path:
    // B dl A.
    if (directlyLinkedEitherDirection(
            prepared,
            metadata,
            fList,
            context.bNode,
            context.aNode
        )) {
        result.valid = true;
        result.segmentPathNodes.push_back(context.bNode);
        result.segmentPathNodes.push_back(context.aNode);
        result.message = "Built direct SEGFO path using FLIST: B -> A.";
        return result;
    }

    std::vector<char> usedNode(
        static_cast<std::size_t>(pathTree.nodes.size()),
        0
    );

    int currentNode = context.bNode;

    result.segmentPathNodes.push_back(context.bNode);
    usedNode[context.bNode] = 1;

    // Linear FLIST scan:
    // extend the current path whenever the next FLIST segment is directly linked
    // to the current path endpoint. This avoids explicit all-pairs SEGGR construction.
    for (int candidateNode : fList.segmentNodes) {
        if (candidateNode < 0 || candidateNode >= static_cast<int>(pathTree.nodes.size())) {
            result.message = "FLIST contains invalid PathTree node id.";
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
                fList,
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
                fList,
                currentNode,
                context.aNode
            );

        if (currentLinkedToA) {
            result.segmentPathNodes.push_back(context.aNode);
            result.valid = true;
            result.message = "Built linear SEGFO path using FLIST: B -> ... -> A.";
            return result;
        }
    }

    result.message =
        "No SEGFO path found by the current FLIST one-pass construction. "
        "The next Williamson step is frontier construction for the fully general case.";

    return result;
}

bool WilliamsonSegfoPathBuilder::directlyLinkedEitherDirection(
    const PreparedPalmTree& prepared,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    int firstNode,
    int secondNode
) {
    return directlyLinkedEarlierLater(
            prepared,
            metadata,
            fList,
            firstNode,
            secondNode
        )
        || directlyLinkedEarlierLater(
            prepared,
            metadata,
            fList,
            secondNode,
            firstNode
        );
}

bool WilliamsonSegfoPathBuilder::directlyLinkedEarlierLater(
    const PreparedPalmTree& prepared,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
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
        fList,
        laterNode,
        earlier.low1Dfs,
        earlier.tailDfsNumber
    );
}

bool WilliamsonSegfoPathBuilder::hasHeadInOpenDfsInterval(
    const PreparedPalmTree& prepared,
    const WilliamsonFList& fList,
    int nodeId,
    int lowExclusiveDfs,
    int highExclusiveDfs
) {
    if (nodeId < 0 || nodeId >= static_cast<int>(fList.headsByNode.size())) {
        return false;
    }

    if (lowExclusiveDfs >= highExclusiveDfs) {
        return false;
    }

    const std::vector<int>& heads =
        fList.headsByNode[nodeId];

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