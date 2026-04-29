#pragma once

#include "ht/certificate/DirectLinkTester.hpp"
#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonBasicCase.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonBasicCaseDetector {
public:
    WilliamsonBasicCase detect(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonContext& context
    ) const;

private:
    static bool directlyLinkedEitherDirection(
        const DirectLinkTester& tester,
        int firstNode,
        int secondNode
    );
};

} // namespace ht