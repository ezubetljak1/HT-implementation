#pragma once

#include <vector>

#include "ht/certificate/HomKernelSignatureBuilder.hpp"
#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class KuratowskiKernelSelector {
public:
    KuratowskiSubdivisionVerification select(
        const PreparedPalmTree& prepared,
        const std::vector<int>& candidateOriginalEdgeIds
    ) const;

private:
    static std::vector<int> expandSkeletonEdges(
        const std::vector<int>& selectedSkeletonEdgeIds,
        const HomKernelSignature& signature
    );

    static const HomKernelSkeletonEdge* findSkeletonEdgeById(
        const HomKernelSignature& signature,
        int skeletonEdgeId
    );
};

} // namespace ht