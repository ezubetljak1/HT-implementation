#pragma once

#include <string>
#include <vector>

#include "ht/certificate/DirectLinkTester.hpp"
#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/PathTreeQueries.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/certificate/WilliamsonKernel.hpp"
#include "ht/certificate/WilliamsonSegfoPath.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonSecondCaseKernelBuilder {
public:
    WilliamsonKernel build(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    ) const;

private:
    struct EdgeCollector {
        const PreparedPalmTree& prepared;
        std::vector<char> seenOriginalEdge;
        std::vector<int> originalEdgeIds;

        explicit EdgeCollector(const PreparedPalmTree& prepared);

        void addDart(int dartId);
        void addDarts(const std::vector<int>& dartIds);
    };

    static int maxOriginalEdgeId(const PreparedPalmTree& prepared);

    static const SegmentMetadata& segmentMetadata(
        const SegmentMetadataTable& metadata,
        int nodeId
    );

    static bool isCleanSecondCaseShape(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    );

    static bool segmentLinksToSpan(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        int sourceNode,
        int spanNode
    );

    static bool addReducedSegment(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        int nodeId,
        EdgeCollector& out
    );

    static bool addLowWitnessPath(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadata& segment,
        const SegmentHeadWitness& witness,
        EdgeCollector& out
    );

    static bool addDirectLinkPath(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        int sourceNode,
        int spanNode,
        EdgeCollector& out
    );
};

} // namespace ht