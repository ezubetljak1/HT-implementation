#pragma once

#include <vector>

namespace ht {

struct WilliamsonFList {
    bool valid = false;

    // FLIST segment nodes in SEGLIST order.
    std::vector<int> segmentNodes;

    // node id -> position in FLIST, or -1.
    std::vector<int> positionByNode;

    // HEAD(Z), materialized only for segments in FLIST.
    // Indexed by path-tree node id.
    std::vector<std::vector<int>> headsByNode;

    // FxLIST[x] = segments Z in FLIST such that x is in HEAD(Z).
    // Indexed by prepared vertex id.
    std::vector<std::vector<int>> fxListByVertex;

    // Optional marker for the segment F.
    int fNode = -1;
    int fPosition = -1;
};

} // namespace ht