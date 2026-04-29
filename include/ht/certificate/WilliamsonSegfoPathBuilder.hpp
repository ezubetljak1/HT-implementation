#pragma once

#include <vector>

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
    struct DfsInterval {
        int startDfs = -1;
        int endDfs = -1;

        bool valid() const {
            return startDfs != -1
                && endDfs != -1
                && startDfs <= endDfs;
        }
    };

    struct DfsRange {
        int startDfs = -1;
        int endDfs = -1;
        int ownerNode = -1;
    };

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

    static DfsInterval intervalForNode(
        const PreparedPalmTree& prepared,
        const SegmentMetadataTable& metadata,
        int nodeId
    );

    static bool tryDiscoverSegment(
        const PathTree& pathTree,
        const WilliamsonFList& fList,
        const WilliamsonContext& context,
        int ownerNode,
        int candidateNode,
        std::vector<char>& visitedNode,
        std::vector<int>& parentNode
    );

    static void enqueueNewIntervalParts(
        const DfsInterval& interval,
        int ownerNode,
        int& coveredMinDfs,
        int& coveredMaxDfs,
        std::vector<DfsRange>& rangeQueue,
        int& rangeTail
    );

    static std::vector<int> buildVertexByDfs(
        const PreparedPalmTree& prepared
    );

    static std::vector<int> reconstructPath(
        int startNode,
        int targetNode,
        const std::vector<int>& parentNode
    );

    static const SegmentMetadata& metadataForNode(
        const SegmentMetadataTable& metadata,
        int nodeId
    );

    static bool isExcludedFromSegfoPath(
        const WilliamsonContext& context,
        int nodeId
    );
};

} // namespace ht