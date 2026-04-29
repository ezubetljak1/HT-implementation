#pragma once

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/certificate/WilliamsonFList.hpp"
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

    WilliamsonSegfoPath buildPathFromFList(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        const WilliamsonContext& context
    ) const;

private:
    static bool directlyLinkedEitherDirection(
        const PreparedPalmTree& prepared,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        int firstNode,
        int secondNode
    );

    static bool directlyLinkedEarlierLater(
        const PreparedPalmTree& prepared,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        int earlierNode,
        int laterNode
    );

    static bool hasHeadInOpenDfsInterval(
        const PreparedPalmTree& prepared,
        const WilliamsonFList& fList,
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