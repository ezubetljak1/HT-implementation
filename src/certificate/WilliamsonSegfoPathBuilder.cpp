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

    std::vector<char> processedDfs(
        static_cast<std::size_t>(prepared.n + 1),
        0
    );

    std::vector<char> visitedNode(
        static_cast<std::size_t>(pathTree.nodes.size()),
        0
    );

    std::vector<int> parentNode(
        static_cast<std::size_t>(pathTree.nodes.size()),
        -1
    );

    // Each discovered segment can extend the covered DFS interval at most twice:
    // once to the left and once to the right. Therefore 2 * nodes + constant is enough.
    std::vector<DfsRange> rangeQueue(
        static_cast<std::size_t>(2 * pathTree.nodes.size() + 4)
    );

    int rangeHead = 0;
    int rangeTail = 0;

    const DfsInterval startInterval =
        intervalForNode(
            prepared,
            metadata,
            context.bNode
        );

    if (!startInterval.valid()) {
        result.message = "B segment has no valid DFS interval for SEGFO traversal.";
        return result;
    }

    int coveredMinDfs = startInterval.startDfs;
    int coveredMaxDfs = startInterval.endDfs;

    visitedNode[context.bNode] = 1;
    rangeQueue[rangeTail++] =
        DfsRange{
            startInterval.startDfs,
            startInterval.endDfs,
            context.bNode
        };

    while (rangeHead < rangeTail) {
        const DfsRange range = rangeQueue[rangeHead++];

        if (range.startDfs < 1 || range.endDfs > prepared.n) {
            result.message = "Invalid DFS range reached during SEGFO frontier traversal.";
            return result;
        }

        for (int dfs = range.startDfs; dfs <= range.endDfs; ++dfs) {
            if (processedDfs[dfs]) {
                continue;
            }

            processedDfs[dfs] = 1;

            const int vertex = vertexByDfs[dfs];

            if (vertex < 0 || vertex >= prepared.n) {
                continue;
            }

            for (int candidateNode : fList.fxListByVertex[vertex]) {
                const bool discovered =
                    tryDiscoverSegment(
                        pathTree,
                        fList,
                        context,
                        range.ownerNode,
                        candidateNode,
                        visitedNode,
                        parentNode
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
                        "Built SEGFO path using explicit FLIST/FxLIST interval frontier.";
                    return result;
                }

                const DfsInterval candidateInterval =
                    intervalForNode(
                        prepared,
                        metadata,
                        candidateNode
                    );

                if (!candidateInterval.valid()) {
                    continue;
                }

                enqueueNewIntervalParts(
                    candidateInterval,
                    candidateNode,
                    coveredMinDfs,
                    coveredMaxDfs,
                    rangeQueue,
                    rangeTail
                );
            }
        }
    }

    result.message =
        "No SEGFO path found by explicit FLIST/FxLIST interval frontier.";

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

WilliamsonSegfoPathBuilder::DfsInterval
WilliamsonSegfoPathBuilder::intervalForNode(
    const PreparedPalmTree& prepared,
    const SegmentMetadataTable& metadata,
    int nodeId
) {
    const SegmentMetadata& segment =
        metadataForNode(metadata, nodeId);

    if (segment.low1Dfs == -1 || segment.tailDfsNumber == -1) {
        return {};
    }

    DfsInterval interval;
    interval.startDfs = segment.low1Dfs + 1;
    interval.endDfs = segment.tailDfsNumber - 1;

    if (interval.startDfs < 1) {
        interval.startDfs = 1;
    }

    if (interval.endDfs > prepared.n) {
        interval.endDfs = prepared.n;
    }

    if (!interval.valid()) {
        return {};
    }

    return interval;
}

bool WilliamsonSegfoPathBuilder::tryDiscoverSegment(
    const PathTree& pathTree,
    const WilliamsonFList& fList,
    const WilliamsonContext& context,
    int ownerNode,
    int candidateNode,
    std::vector<char>& visitedNode,
    std::vector<int>& parentNode
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
    parentNode[candidateNode] = ownerNode;

    return true;
}

void WilliamsonSegfoPathBuilder::enqueueNewIntervalParts(
    const DfsInterval& interval,
    int ownerNode,
    int& coveredMinDfs,
    int& coveredMaxDfs,
    std::vector<DfsRange>& rangeQueue,
    int& rangeTail
) {
    if (!interval.valid()) {
        return;
    }

    const int oldMin = coveredMinDfs;
    const int oldMax = coveredMaxDfs;

    if (interval.startDfs < oldMin) {
        if (rangeTail < static_cast<int>(rangeQueue.size())) {
            rangeQueue[rangeTail++] =
                DfsRange{
                    interval.startDfs,
                    oldMin - 1,
                    ownerNode
                };
        }

        coveredMinDfs = interval.startDfs;
    }

    if (interval.endDfs > oldMax) {
        if (rangeTail < static_cast<int>(rangeQueue.size())) {
            rangeQueue[rangeTail++] =
                DfsRange{
                    oldMax + 1,
                    interval.endDfs,
                    ownerNode
                };
        }

        coveredMaxDfs = interval.endDfs;
    }
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