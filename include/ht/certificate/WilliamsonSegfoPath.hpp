#pragma once

#include <string>
#include <vector>

namespace ht {

struct WilliamsonSegfoPath {
    bool valid = false;

    // Segment path in SEGFO(e, X), from B to A.
    // Examples:
    //   direct case: B, A
    //   one-intermediate case: B, Y, A
    std::vector<int> segmentPathNodes;

    std::string message;
};

} // namespace ht