#pragma once

#include <string>
#include <vector>

namespace ht {

struct WilliamsonSegmentList {
    bool valid = false;

    // PathTree node for the base PATH(e), whose children form SEGLIST(e).
    int baseNode = -1;

    // Nodes in SEGLIST(e), in PathTree child order.
    std::vector<int> segmentNodes;

    // node id -> position in segmentNodes, or -1.
    std::vector<int> positionByNode;

    int fPosition = -1;
    int aPosition = -1;
    int bPosition = -1;

    std::string message;
};

} // namespace ht