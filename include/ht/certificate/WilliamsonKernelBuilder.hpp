#pragma once

#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/PathTreeQueries.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/certificate/WilliamsonKernel.hpp"
#include "ht/certificate/WilliamsonSegfoPath.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonKernelBuilder {
public:
    WilliamsonKernel buildKernelFromSegfoPath(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    ) const;

private:
    static int maxOriginalEdgeId(const PreparedPalmTree& prepared);

    static void addUniqueOriginalEdgeFromDart(
        const PreparedPalmTree& prepared,
        int dartId,
        std::vector<char>& seenEdge,
        std::vector<int>& originalEdgeIds
    );

    static void addUniqueOriginalEdgesFromDarts(
        const PreparedPalmTree& prepared,
        const std::vector<int>& dartIds,
        std::vector<char>& seenEdge,
        std::vector<int>& originalEdgeIds
    );

    static void addCycleForNode(
        const PreparedPalmTree& prepared,
        const PathTreeQueries& queries,
        int nodeId,
        std::vector<char>& seenEdge,
        std::vector<int>& originalEdgeIds
    );

    static void addSegmentSubtreeForNode(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        int nodeId,
        std::vector<char>& seenPathNode,
        std::vector<char>& seenEdge,
        std::vector<int>& originalEdgeIds
    );
};

} // namespace ht