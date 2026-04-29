#pragma once

#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/PathTreeQueries.hpp"
#include "ht/certificate/WilliamsonBasicCase.hpp"
#include "ht/certificate/WilliamsonKernel.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonKernelBuilder {
public:
    WilliamsonKernel buildDirectKernel(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const WilliamsonBasicCase& basicCase
    ) const;

private:
    static int maxOriginalEdgeId(const PreparedPalmTree& prepared);

    static void addUniqueOriginalEdgeFromDart(
        const PreparedPalmTree& prepared,
        int dartId,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );

    static void addUniqueOriginalEdgesFromDarts(
        const PreparedPalmTree& prepared,
        const std::vector<int>& dartIds,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );

    static void addCycleForNode(
        const PreparedPalmTree& prepared,
        const PathTreeQueries& queries,
        int nodeId,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );

    static void addSegmentForNode(
        const PreparedPalmTree& prepared,
        const PathTreeQueries& queries,
        int nodeId,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );

    static void addKernelNodeContribution(
        const PreparedPalmTree& prepared,
        const PathTreeQueries& queries,
        int nodeId,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );
};

} // namespace ht