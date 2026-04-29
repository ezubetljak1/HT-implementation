#pragma once

#include "ht/certificate/DirectLinkTester.hpp"
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
    static int positionOf(
        const std::vector<int>& positionByNode,
        int nodeId
    );
};

} // namespace ht