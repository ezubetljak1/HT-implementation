#pragma once

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
};

} // namespace ht