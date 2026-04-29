#pragma once

#include <string>
#include <vector>

#include "ht/certificate/WilliamsonContext.hpp"

namespace ht {

enum class WilliamsonBasicCaseType {
    None,
    DirectTriangleLinks
};

struct WilliamsonBasicCase {
    bool valid = false;

    WilliamsonBasicCaseType type = WilliamsonBasicCaseType::None;

    WilliamsonContext context;

    // Segment path from B to A in the reduced Williamson case.
    // For basic case 1 this is simply: B, A.
    std::vector<int> segmentPathNodes;

    std::string message;
};

} // namespace ht