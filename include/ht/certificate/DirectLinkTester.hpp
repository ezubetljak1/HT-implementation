#pragma once

#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class DirectLinkTester {
public:
    DirectLinkTester(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadataTable
    );

    // Targeted HEAD(SEG(nodeId)).
    // This is not precomputed for all nodes to avoid non-linear eager materialization.
    std::vector<int> headVerticesForNode(int nodeId) const;

    // Williamson check used when earlierNode is F and laterNode is Y_i:
    // does HEAD(Y_i) contain a vertex in OSPAN(F), i.e. between LOW1(F) and TAIL(F)?
    bool directlyLinkedToEarlierSegment(
        int earlierNodeId,
        int laterNodeId
    ) const;

    bool hasHeadInOpenDfsInterval(
        int nodeId,
        int lowExclusiveDfs,
        int highExclusiveDfs
    ) const;

private:
    const PreparedPalmTree& prepared_;
    const PathTree& pathTree_;
    const SegmentMetadataTable& metadataTable_;

    const Dart& dart(int dartId) const;
    const SegmentMetadata& metadata(int nodeId) const;

    void collectHeadVerticesFromSubtree(
        int nodeId,
        std::vector<char>& seenVertex,
        std::vector<int>& headVertices
    ) const;

    void addHeadVertexFromDart(
        int dartId,
        std::vector<char>& seenVertex,
        std::vector<int>& headVertices
    ) const;
};

} // namespace ht