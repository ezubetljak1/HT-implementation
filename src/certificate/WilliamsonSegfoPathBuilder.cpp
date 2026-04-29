#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"

#include "ht/certificate/WilliamsonFListBuilder.hpp"

#include <algorithm>
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

    // Direct SEGFO edge: B dl A.
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

    std::vector<int> vertexByDfs =
        buildVertexByDfs(prepared);

    std::vector<int> nextDfs(
        static_cast<std::size_t>(prepared.n + 2),
        0
    );

    for (int i = 0; i < static_cast<int>(nextDfs.size()); ++i) {
        nextDfs[i] = i;
    }

    std::vector<char> visitedNode(
        static_cast<std::size_t>(pathTree.nodes.size()),
        0
    );

    std::vector<int> parentNode(
        static_cast<std::size_t>(pathTree.nodes.size()),
        -1
    );

    std::vector<int> queue(
        static_cast<std::size_t>(pathTree.nodes.size()),
        -1
    );

    int queueHead = 0;
    int queueTail = 0;

    visitedNode[context.bNode] = 1;
    queue[queueTail++] = context.bNode;

    while (queueHead < queueTail) {
        const int currentNode = queue[queueHead++];

        if (currentNode < 0 || currentNode >= static_cast<int>(pathTree.nodes.size())) {
            result.message = "Invalid node reached during SEGFO frontier traversal.";
            return result;
        }

        const SegmentMetadata& currentMetadata =
            metadataForNode(metadata, currentNode);

        if (currentMetadata.low1Dfs == -1 || currentMetadata.tailDfsNumber == -1) {
            continue;
        }

        int startDfs = currentMetadata.low1Dfs + 1;
        int endDfs = currentMetadata.tailDfsNumber - 1;

        if (startDfs < 1) {
            startDfs = 1;
        }

        if (endDfs > prepared.n) {
            endDfs = prepared.n;
        }

        int dfs = findNextDfs(nextDfs, startDfs);

        while (dfs <= endDfs) {
            const int currentDfs = dfs;

            markDfsProcessed(nextDfs, currentDfs);

            if (currentDfs >= 0
                && currentDfs < static_cast<int>(vertexByDfs.size())) {
                const int vertex = vertexByDfs[currentDfs];

                if (vertex >= 0 && vertex < prepared.n) {
                    for (int candidateNode : fList.fxListByVertex[vertex]) {
                        const bool discovered =
                            tryDiscoverSegment(
                                pathTree,
                                fList,
                                context,
                                currentNode,
                                candidateNode,
                                visitedNode,
                                parentNode,
                                queue,
                                queueTail
                            );

                        if (!discovered) {
                            continue;
                        }

                        if (candidateNode == context.aNode) {
                            result.valid = true;
                            result.segmentPathNodes =
                                reconstructPath(
                                    context.bNode,
                                    context.aNode,
                                    parentNode
                                );

                            result.message =
                                "Built SEGFO path using FLIST/FxLIST frontier.";
                            return result;
                        }
                    }
                }
            }

            dfs = findNextDfs(nextDfs, currentDfs);
        }
    }

    result.message =
        "No SEGFO path found by FLIST/FxLIST frontier construction.";

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

bool WilliamsonSegfoPathBuilder::tryDiscoverSegment(
    const PathTree& pathTree,
    const WilliamsonFList& fList,
    const WilliamsonContext& context,
    int currentNode,
    int candidateNode,
    std::vector<char>& visitedNode,
    std::vector<int>& parentNode,
    std::vector<int>& queue,
    int& queueTail
) {
    if (candidateNode < 0 || candidateNode >= static_cast<int>(pathTree.nodes.size())) {
        return false;
    }

    if (candidateNode >= static_cast<int>(fList.positionByNode.size())
        || fList.positionByNode[candidateNode] == -1) {
        return false;
    }

    if (isExcludedFromSegfoPath(context, candidateNode)) {
        return false;
    }

    if (visitedNode[candidateNode]) {
        return false;
    }

    visitedNode[candidateNode] = 1;
    parentNode[candidateNode] = currentNode;

    if (queueTail < static_cast<int>(queue.size())) {
        queue[queueTail++] = candidateNode;
    }

    return true;
}

std::vector<int> WilliamsonSegfoPathBuilder::buildVertexByDfs(
    const PreparedPalmTree& prepared
) {
    std::vector<int> vertexByDfs(
        static_cast<std::size_t>(prepared.n + 1),
        -1
    );

    for (int vertex = 0; vertex < prepared.n; ++vertex) {
        const int dfs = prepared.number[vertex];

        if (dfs >= 0 && dfs < static_cast<int>(vertexByDfs.size())) {
            vertexByDfs[dfs] = vertex;
        }
    }

    return vertexByDfs;
}

int WilliamsonSegfoPathBuilder::findNextDfs(
    std::vector<int>& nextDfs,
    int value
) {
    if (value < 0) {
        value = 0;
    }

    if (value >= static_cast<int>(nextDfs.size())) {
        return static_cast<int>(nextDfs.size());
    }

    if (nextDfs[value] == value) {
        return value;
    }

    nextDfs[value] =
        findNextDfs(
            nextDfs,
            nextDfs[value]
        );

    return nextDfs[value];
}

void WilliamsonSegfoPathBuilder::markDfsProcessed(
    std::vector<int>& nextDfs,
    int value
) {
    if (value < 0 || value >= static_cast<int>(nextDfs.size())) {
        return;
    }

    nextDfs[value] =
        findNextDfs(
            nextDfs,
            value + 1
        );
}

std::vector<int> WilliamsonSegfoPathBuilder::reconstructPath(
    int startNode,
    int targetNode,
    const std::vector<int>& parentNode
) {
    std::vector<int> reversedPath;

    int current = targetNode;

    while (current != -1) {
        reversedPath.push_back(current);

        if (current == startNode) {
            break;
        }

        if (current < 0 || current >= static_cast<int>(parentNode.size())) {
            return {};
        }

        current = parentNode[current];
    }

    if (reversedPath.empty() || reversedPath.back() != startNode) {
        return {};
    }

    std::reverse(
        reversedPath.begin(),
        reversedPath.end()
    );

    return reversedPath;
}

const SegmentMetadata& WilliamsonSegfoPathBuilder::metadataForNode(
    const SegmentMetadataTable& metadata,
    int nodeId
) {
    if (nodeId < 0 || nodeId >= static_cast<int>(metadata.segmentByNode.size())) {
        throw std::runtime_error("Invalid node id while reading SegmentMetadata.");
    }

    const int segmentId =
        metadata.segmentByNode[nodeId];

    if (segmentId < 0 || segmentId >= static_cast<int>(metadata.segments.size())) {
        throw std::runtime_error("Invalid segment id while reading SegmentMetadata.");
    }

    return metadata.segments[segmentId];
}

bool WilliamsonSegfoPathBuilder::isExcludedFromSegfoPath(
    const WilliamsonContext& context,
    int nodeId
) {
    return nodeId == context.fNode
        || nodeId == context.bNode
        || nodeId == context.cycleNode;
}

} // namespace ht