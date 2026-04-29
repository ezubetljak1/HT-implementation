#pragma once

#include <string>
#include <vector>

#include "ht/certificate/WilliamsonBasicCase.hpp"

namespace ht {

struct WilliamsonKernel {
    bool valid = false;

    WilliamsonBasicCase basicCase;

    // Original graph edge IDs belonging to the materialized kernel candidate.
    std::vector<int> originalEdgeIds;

    std::string message;
};

} // namespace ht