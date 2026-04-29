#pragma once

#include <string>
#include <vector>

namespace ht {

struct HomKernelLookupResult {
    bool found = false;

    // Runtime table returns actual skeleton edge IDs from HomKernelSignature.
    std::vector<int> selectedSkeletonEdgeIds;

    std::string message;
};

class HomKernelLookupTable {
public:
    HomKernelLookupResult lookup(
        const std::string& shapeKey
    ) const;
};

} // namespace ht