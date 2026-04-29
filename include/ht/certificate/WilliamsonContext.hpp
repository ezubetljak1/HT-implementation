#pragma once

#include <string>

namespace ht {

struct WilliamsonContext {
    bool valid = false;

    // Node in PathTree corresponding to the segment F.
    int fNode = -1;

    // Nodes in PathTree corresponding to A and B.
    int aNode = -1;
    int bNode = -1;

    // Node/dart for the base cycle CYCLE(e) from the strong-planarity failure.
    int cycleNode = -1;
    int cycleDart = -1;

    // Defining darts for F, A and B.
    int fDart = -1;
    int aDart = -1;
    int bDart = -1;

    // Parent path-tree node, useful later for SEGLIST(e).
    int parentNode = -1;

    bool aLinkedToF = false;
    bool bLinkedToF = false;

    std::string message;
};

} // namespace ht