#pragma once

#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

struct DirectLinkWitness {
    bool exists = false;

    // HEAD(SEG(sourceNode)) intersects OSPAN(spanNode).
    int sourceNode = -1;
    int spanNode = -1;

    // Concrete back dart proving the direct link.
    int backDart = -1;
    int backTailVertex = -1;
    int headVertex = -1;
    int headDfs = -1;
};

class DirectLinkTester {
public:
    DirectLinkTester(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadataTable
    );

    // Old API kept for compatibility.
    std::vector<int> headVerticesForNode(int nodeId) const;

    // New API: returns concrete HEAD witnesses from SEG(nodeId).
    std::vector<SegmentHeadWitness> headWitnessesForNode(int nodeId) const;

    // Old API kept for compatibility.
    bool directlyLinkedToEarlierSegment(
        int earlierNodeId,
        int laterNodeId
    ) const;

    // Old API kept for compatibility.
    bool hasHeadInOpenDfsInterval(
        int nodeId,
        int lowExclusiveDfs,
        int highExclusiveDfs
    ) const;

    // New API: finds a concrete witness that HEAD(SEG(nodeId))
    // intersects the open DFS interval (lowExclusiveDfs, highExclusiveDfs).
    SegmentHeadWitness findHeadWitnessInOpenDfsInterval(
        int nodeId,
        int lowExclusiveDfs,
        int highExclusiveDfs
    ) const;

    // New API: returns the concrete back dart proving:
    //
    //     SEG(laterNodeId) dl SEG(earlierNodeId)
    //
    // meaning HEAD(SEG(laterNodeId)) intersects OSPAN(earlierNodeId).
    DirectLinkWitness findDirectLinkToEarlierSegment(
        int earlierNodeId,
        int laterNodeId
    ) const;

    // Same as above, but named for kernel construction:
    //
    //     sourceNode has a back edge into OSPAN(spanNode).
    DirectLinkWitness findPathFromSegmentToOpenSpan(
        int sourceNodeId,
        int spanNodeId
    ) const;

private:
    const PreparedPalmTree& prepared_;
    const PathTree& pathTree_;
    const SegmentMetadataTable& metadataTable_;

    const Dart& dart(int dartId) const;
    const SegmentMetadata& metadata(int nodeId) const;

    void collectHeadWitnessesFromSubtree(
        int nodeId,
        std::vector<char>& seenVertex,
        std::vector<SegmentHeadWitness>& witnesses
    ) const;

    void addHeadWitnessFromDart(
        int dartId,
        std::vector<char>& seenVertex,
        std::vector<SegmentHeadWitness>& witnesses
    ) const;
};

} // namespace ht