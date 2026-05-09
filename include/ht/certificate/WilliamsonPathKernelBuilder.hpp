#pragma once

#include <string>
#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/PathTreeQueries.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/certificate/WilliamsonFList.hpp"
#include "ht/certificate/WilliamsonFListBuilder.hpp"
#include "ht/certificate/WilliamsonKernel.hpp"
#include "ht/certificate/WilliamsonLinkOracle.hpp"
#include "ht/certificate/WilliamsonSegfoPath.hpp"
#include "ht/certificate/WilliamsonSegmentList.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonPathKernelBuilder {
public:
    WilliamsonKernel build(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    ) const;

    WilliamsonKernel build(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    ) const;

    WilliamsonKernel buildBasicCase1(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    ) const;

    WilliamsonKernel buildBasicCase1(
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

    struct NormalizedPath {
        bool valid = false;
        bool isBasicCase1 = false;
        bool isBasicCase2 = false;

        WilliamsonContext context;
        WilliamsonSegfoPath segfoPath;

        std::string message;
    };

    static int maxOriginalEdgeId(const PreparedPalmTree& prepared);

    static const SegmentMetadata& segmentMetadata(
        const SegmentMetadataTable& metadata,
        int nodeId
    );

    static NormalizedPath normalizeToBasicCase(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    );

    static bool internalNodeLinksToF(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        int nodeId,
        int fNode
    );

    static WilliamsonContext makeReducedContext(
        const WilliamsonContext& oldContext,
        int newBNode,
        int newANode
    );

    static WilliamsonSegfoPath makeReducedPath(
        const std::vector<int>& oldPath,
        int startIndex,
        int endIndex
    );

    static bool isBasicCase1(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    );

    static bool isCleanBasicCase2(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
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
        const WilliamsonFList& fList,
        int sourceNode,
        int spanNode,
        EdgeCollector& out
    );
};

} // namespace ht