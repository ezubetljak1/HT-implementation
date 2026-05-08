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
    const std::vector<SegmentHeadWitness> witnesses =
        headWitnessesForNode(nodeId);

    std::vector<int> vertices;
    vertices.reserve(witnesses.size());

    for (const SegmentHeadWitness& witness : witnesses) {
        vertices.push_back(witness.headVertex);
    }

    return vertices;
}

std::vector<SegmentHeadWitness> DirectLinkTester::headWitnessesForNode(
    int nodeId
) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(pathTree_.nodes.size())) {
        throw std::runtime_error("Invalid path-tree node id in headWitnessesForNode.");
    }

    std::vector<char> seenVertex(static_cast<std::size_t>(prepared_.n), 0);
    std::vector<SegmentHeadWitness> witnesses;

    collectHeadWitnessesFromSubtree(
        nodeId,
        seenVertex,
        witnesses
    );

    return witnesses;
}

void DirectLinkTester::collectHeadWitnessesFromSubtree(
    int nodeId,
    std::vector<char>& seenVertex,
    std::vector<SegmentHeadWitness>& witnesses
) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(pathTree_.nodes.size())) {
        throw std::runtime_error("Invalid node id while collecting head witnesses.");
    }

    const PathNode& node = pathTree_.nodes[nodeId];

    for (int dartId : node.pathDarts) {
        addHeadWitnessFromDart(
            dartId,
            seenVertex,
            witnesses
        );
    }

    for (int childNodeId : node.children) {
        collectHeadWitnessesFromSubtree(
            childNodeId,
            seenVertex,
            witnesses
        );
    }
}

void DirectLinkTester::addHeadWitnessFromDart(
    int dartId,
    std::vector<char>& seenVertex,
    std::vector<SegmentHeadWitness>& witnesses
) const {
    const Dart& d = dart(dartId);

    // HEAD contribution comes only from active directed back edge:
    //
    //     backTailVertex -> headVertex
    //
    // This concrete dart is exactly what we need later for Kuratowski paths.
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

    const int headDfs = prepared_.number[headVertex];

    if (headDfs <= 0) {
        return;
    }

    seenVertex[headVertex] = 1;

    SegmentHeadWitness witness;
    witness.headVertex = headVertex;
    witness.headDfs = headDfs;
    witness.backDart = dartId;
    witness.backTailVertex = d.from;

    witnesses.push_back(witness);
}

bool DirectLinkTester::hasHeadInOpenDfsInterval(
    int nodeId,
    int lowExclusiveDfs,
    int highExclusiveDfs
) const {
    return findHeadWitnessInOpenDfsInterval(
        nodeId,
        lowExclusiveDfs,
        highExclusiveDfs
    ).backDart != -1;
}

SegmentHeadWitness DirectLinkTester::findHeadWitnessInOpenDfsInterval(
    int nodeId,
    int lowExclusiveDfs,
    int highExclusiveDfs
) const {
    SegmentHeadWitness empty;

    if (lowExclusiveDfs >= highExclusiveDfs) {
        return empty;
    }

    const std::vector<SegmentHeadWitness> witnesses =
        headWitnessesForNode(nodeId);

    for (const SegmentHeadWitness& witness : witnesses) {
        if (witness.headDfs > lowExclusiveDfs &&
            witness.headDfs < highExclusiveDfs) {
            return witness;
        }
    }

    return empty;
}

bool DirectLinkTester::directlyLinkedToEarlierSegment(
    int earlierNodeId,
    int laterNodeId
) const {
    return findDirectLinkToEarlierSegment(
        earlierNodeId,
        laterNodeId
    ).exists;
}

DirectLinkWitness DirectLinkTester::findDirectLinkToEarlierSegment(
    int earlierNodeId,
    int laterNodeId
) const {
    return findPathFromSegmentToOpenSpan(
        laterNodeId,
        earlierNodeId
    );
}

DirectLinkWitness DirectLinkTester::findPathFromSegmentToOpenSpan(
    int sourceNodeId,
    int spanNodeId
) const {
    DirectLinkWitness result;
    result.sourceNode = sourceNodeId;
    result.spanNode = spanNodeId;

    const SegmentMetadata& span = metadata(spanNodeId);

    if (span.low1Dfs == -1 || span.tailDfsNumber == -1) {
        return result;
    }

    SegmentHeadWitness witness =
        findHeadWitnessInOpenDfsInterval(
            sourceNodeId,
            span.low1Dfs,
            span.tailDfsNumber
        );

    if (witness.backDart == -1) {
        return result;
    }

    result.exists = true;
    result.backDart = witness.backDart;
    result.backTailVertex = witness.backTailVertex;
    result.headVertex = witness.headVertex;
    result.headDfs = witness.headDfs;

    return result;
}

} // namespace ht