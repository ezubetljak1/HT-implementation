#pragma once

#include <vector>

#include "ht/certificate/DirectLinkTester.hpp"
#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/certificate/WilliamsonSegmentList.hpp"
#include "ht/certificate/WilliamsonSegfoPath.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonSegfoPathBuilder {
public:
    WilliamsonSegfoPath buildPath(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonSegmentList& segmentList,
        const WilliamsonContext& context
    ) const;

private:
    struct HeadCache {
        std::vector<std::vector<int>> headsByNode;
    };

    static HeadCache buildHeadCache(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonSegmentList& segmentList
    );

    static bool directlyLinkedEitherDirection(
        const PreparedPalmTree& prepared,
        const SegmentMetadataTable& metadata,
        const HeadCache& headCache,
        int firstNode,
        int secondNode
    );

    static bool directlyLinkedEarlierLater(
        const PreparedPalmTree& prepared,
        const SegmentMetadataTable& metadata,
        const HeadCache& headCache,
        int earlierNode,
        int laterNode
    );

    static bool hasHeadInOpenDfsInterval(
        const PreparedPalmTree& prepared,
        const HeadCache& headCache,
        int nodeId,
        int lowExclusiveDfs,
        int highExclusiveDfs
    );

    static const SegmentMetadata& metadataForNode(
        const SegmentMetadataTable& metadata,
        int nodeId
    );

    static bool isContextNode(
        const WilliamsonContext& context,
        int nodeId
    );
};

} // namespace ht