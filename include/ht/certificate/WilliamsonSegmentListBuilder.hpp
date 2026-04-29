#pragma once

#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/certificate/WilliamsonSegmentList.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonSegmentListBuilder {
public:
    WilliamsonSegmentList build(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const WilliamsonContext& context
    ) const;

private:
    static int positionOf(
        const std::vector<int>& positionByNode,
        int nodeId
    );

    static void addUniqueCycleVertex(
        const PreparedPalmTree& prepared,
        int vertex,
        std::vector<char>& seenVertex,
        std::vector<int>& cycleVertices
    );

    static void addUniqueSegmentNode(
        const PathTree& pathTree,
        int nodeId,
        std::vector<char>& seenNode,
        std::vector<int>& segmentNodes
    );

    static void addNodeForDartAndReverse(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        int dartId,
        const std::vector<char>& dartIsOnBaseCycle,
        std::vector<char>& seenNode,
        std::vector<int>& segmentNodes
    );

    static int nodeForDart(
        const PathTree& pathTree,
        int dartId
    );
};

} // namespace ht