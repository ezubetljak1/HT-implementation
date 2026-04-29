#pragma once

#include <string>
#include <vector>

#include "ht/certificate/WilliamsonBasicCase.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/certificate/WilliamsonSegfoPath.hpp"

namespace ht {

struct WilliamsonKernel {
    bool valid = false;

    WilliamsonContext context;
    WilliamsonBasicCase basicCase;
    WilliamsonSegfoPath segfoPath;

    // Original graph edge IDs belonging to the materialized kernel candidate.
    std::vector<int> originalEdgeIds;

    std::string message;
};

} // namespace ht