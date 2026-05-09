#pragma once

#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonFList.hpp"
#include "ht/certificate/WilliamsonSegmentList.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonFListBuilder {
public:
    WilliamsonFList buildFromSegmentList(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonSegmentList& segmentList,
        int fNode
    ) const;

private:
    struct ActiveSegment {
        int nodeId = -1;
        int subtreeEnd = -1;
    };

    static int positionOf(
        const std::vector<int>& positionByNode,
        int nodeId
    );

    static void addWitnessToSegment(
        const PreparedPalmTree& prepared,
        WilliamsonFList& result,
        int segmentNode,
        const SegmentHeadWitness& witness
    );

    static void addWitnessesFromPathNode(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        int pathNodeId,
        std::vector<SegmentHeadWitness>& witnesses
    );

    static SegmentHeadWitness witnessFromBackDart(
        const PreparedPalmTree& prepared,
        int dartId
    );
};

} // namespace ht