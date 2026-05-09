#include "ht/certificate/WilliamsonLinkOracle.hpp"

#include <stdexcept>

namespace ht {

WilliamsonLinkOracle::WilliamsonLinkOracle(
    const PreparedPalmTree& prepared,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList
) : prepared_(prepared),
    metadata_(metadata),
    fList_(fList) {}

const SegmentMetadata& WilliamsonLinkOracle::metadataForNode(
    int nodeId
) const {
    if (nodeId < 0 ||
        nodeId >= static_cast<int>(metadata_.segmentByNode.size())) {
        throw std::runtime_error(
            "Invalid node id in WilliamsonLinkOracle."
        );
    }

    const int segmentId = metadata_.segmentByNode[nodeId];

    if (segmentId < 0 ||
        segmentId >= static_cast<int>(metadata_.segments.size())) {
        throw std::runtime_error(
            "Invalid segment id in WilliamsonLinkOracle."
        );
    }

    return metadata_.segments[segmentId];
}

WilliamsonLinkWitness WilliamsonLinkOracle::findLinkToOpenSpan(
    int sourceNode,
    int spanNode
) const {
    WilliamsonLinkWitness result;
    result.sourceNode = sourceNode;
    result.spanNode = spanNode;

    if (sourceNode < 0 ||
        sourceNode >= static_cast<int>(fList_.headWitnessesByNode.size())) {
        return result;
    }

    const SegmentMetadata& span =
        metadataForNode(spanNode);

    if (span.low1Dfs == -1 || span.tailDfsNumber == -1) {
        return result;
    }

    const int lowExclusiveDfs = span.low1Dfs;
    const int highExclusiveDfs = span.tailDfsNumber;

    if (lowExclusiveDfs >= highExclusiveDfs) {
        return result;
    }

    const std::vector<SegmentHeadWitness>& witnesses =
        fList_.headWitnessesByNode[sourceNode];

    for (const SegmentHeadWitness& witness : witnesses) {
        if (witness.headVertex < 0 ||
            witness.headVertex >= prepared_.n ||
            witness.backDart < 0 ||
            witness.backTailVertex < 0) {
            continue;
        }

        const int headDfs = witness.headDfs;

        if (headDfs > lowExclusiveDfs &&
            headDfs < highExclusiveDfs) {
            result.exists = true;
            result.backDart = witness.backDart;
            result.backTailVertex = witness.backTailVertex;
            result.headVertex = witness.headVertex;
            result.headDfs = witness.headDfs;
            return result;
        }
    }

    return result;
}

bool WilliamsonLinkOracle::linksToOpenSpan(
    int sourceNode,
    int spanNode
) const {
    return findLinkToOpenSpan(
        sourceNode,
        spanNode
    ).exists;
}

} // namespace ht