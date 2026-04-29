#include "ht/certificate/DirectLinkTester.hpp"

#include <stdexcept>

namespace ht {

DirectLinkTester::DirectLinkTester(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadataTable
) : prepared_(prepared),
    pathTree_(pathTree),
    metadataTable_(metadataTable) {}

const Dart& DirectLinkTester::dart(int dartId) const {
    if (dartId < 0 || dartId >= static_cast<int>(prepared_.darts.size())) {
        throw std::runtime_error("Invalid dart id in DirectLinkTester.");
    }

    return prepared_.darts[dartId];
}

const SegmentMetadata& DirectLinkTester::metadata(int nodeId) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(metadataTable_.segmentByNode.size())) {
        throw std::runtime_error("Invalid node id in DirectLinkTester.");
    }

    const int segmentId = metadataTable_.segmentByNode[nodeId];

    if (segmentId < 0 || segmentId >= static_cast<int>(metadataTable_.segments.size())) {
        throw std::runtime_error("Invalid segment id in DirectLinkTester.");
    }

    return metadataTable_.segments[segmentId];
}

std::vector<int> DirectLinkTester::headVerticesForNode(int nodeId) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(pathTree_.nodes.size())) {
        throw std::runtime_error("Invalid path-tree node id in headVerticesForNode.");
    }

    std::vector<char> seenVertex(static_cast<std::size_t>(prepared_.n), 0);
    std::vector<int> headVertices;

    collectHeadVerticesFromSubtree(
        nodeId,
        seenVertex,
        headVertices
    );

    return headVertices;
}

void DirectLinkTester::collectHeadVerticesFromSubtree(
    int nodeId,
    std::vector<char>& seenVertex,
    std::vector<int>& headVertices
) const {
    const PathNode& node = pathTree_.nodes[nodeId];

    for (int dartId : node.pathDarts) {
        addHeadVertexFromDart(
            dartId,
            seenVertex,
            headVertices
        );
    }

    for (int childNodeId : node.children) {
        if (childNodeId < 0 || childNodeId >= static_cast<int>(pathTree_.nodes.size())) {
            throw std::runtime_error("Invalid child node id in DirectLinkTester.");
        }

        collectHeadVerticesFromSubtree(
            childNodeId,
            seenVertex,
            headVertices
        );
    }
}

void DirectLinkTester::addHeadVertexFromDart(
    int dartId,
    std::vector<char>& seenVertex,
    std::vector<int>& headVertices
) const {
    const Dart& d = dart(dartId);

    // HEAD contribution comes from directed back edge f=(tail, head).
    if (!d.isBack) {
        return;
    }

    const int headVertex = d.to;

    if (headVertex < 0 || headVertex >= prepared_.n) {
        return;
    }

    if (seenVertex[headVertex]) {
        return;
    }

    seenVertex[headVertex] = 1;
    headVertices.push_back(headVertex);
}

bool DirectLinkTester::hasHeadInOpenDfsInterval(
    int nodeId,
    int lowExclusiveDfs,
    int highExclusiveDfs
) const {
    if (lowExclusiveDfs >= highExclusiveDfs) {
        return false;
    }

    const std::vector<int> heads =
        headVerticesForNode(nodeId);

    for (int vertex : heads) {
        if (vertex < 0 || vertex >= prepared_.n) {
            continue;
        }

        const int dfs = prepared_.number[vertex];

        if (dfs > lowExclusiveDfs && dfs < highExclusiveDfs) {
            return true;
        }
    }

    return false;
}

bool DirectLinkTester::directlyLinkedToEarlierSegment(
    int earlierNodeId,
    int laterNodeId
) const {
    const SegmentMetadata& earlier = metadata(earlierNodeId);

    if (earlier.low1Dfs == -1 || earlier.tailDfsNumber == -1) {
        return false;
    }

    return hasHeadInOpenDfsInterval(
        laterNodeId,
        earlier.low1Dfs,
        earlier.tailDfsNumber
    );
}

} // namespace ht