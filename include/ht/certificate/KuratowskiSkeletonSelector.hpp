#pragma once

#include <string>
#include <vector>

#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class KuratowskiSkeletonSelector {
public:
    KuratowskiSubdivisionVerification select(
        const PreparedPalmTree& prepared,
        const std::vector<int>& candidateOriginalEdgeIds
    ) const;

private:
    static std::vector<int> deduplicateOriginalEdgeIds(
        const PreparedPalmTree& prepared,
        const std::vector<int>& candidateOriginalEdgeIds
    );

    static int maxOriginalEdgeId(
        const PreparedPalmTree& prepared
    );

    static bool tryBoundedOriginalEdgeSubsets(
        const PreparedPalmTree& prepared,
        const std::vector<int>& candidateOriginalEdgeIds,
        KuratowskiSubdivisionVerification& result
    );

    static bool backtrackOriginalEdgeSubsets(
        const PreparedPalmTree& prepared,
        const std::vector<int>& candidateOriginalEdgeIds,
        int subsetSize,
        int startIndex,
        std::vector<int>& selectedOriginalEdgeIds,
        KuratowskiSubdivisionVerification& result
    );
};

} // namespace ht