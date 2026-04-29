#pragma once

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
    static bool directlyLinkedEitherDirection(
        const DirectLinkTester& tester,
        int firstNode,
        int secondNode
    );

    static bool isContextNode(
        const WilliamsonContext& context,
        int nodeId
    );
};

} // namespace ht