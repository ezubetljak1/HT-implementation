#include "ht/certificate/KuratowskiKernelSelector.hpp"

#include "ht/certificate/HomKernelLookupTable.hpp"
#include "ht/certificate/HomKernelSignatureBuilder.hpp"

namespace ht {

KuratowskiSubdivisionVerification KuratowskiKernelSelector::select(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds
) const {
    KuratowskiSubdivisionVerification result;

    KuratowskiSubdivisionVerifier verifier;
    KuratowskiSubdivisionVerification direct =
        verifier.verify(
            prepared,
            candidateOriginalEdgeIds
        );

    if (direct.valid) {
        return direct;
    }

    HomKernelSignatureBuilder signatureBuilder;
    HomKernelSignature signature =
        signatureBuilder.build(
            prepared,
            candidateOriginalEdgeIds
        );

    if (!signature.valid) {
        result.message = signature.message;
        return result;
    }

    HomKernelLookupTable lookupTable;
    HomKernelLookupResult lookup =
        lookupTable.lookup(signature.shapeKey);

    if (!lookup.found) {
        result.message =
            lookup.message + " Shape key = " + signature.shapeKey;
        return result;
    }

    std::vector<int> selectedOriginalEdgeIds =
        expandSkeletonEdges(
            lookup.selectedSkeletonEdgeIds,
            signature
        );

    if (selectedOriginalEdgeIds.empty()) {
        result.message =
            "HOMKERNEL lookup produced an empty selected edge set.";
        return result;
    }

    KuratowskiSubdivisionVerification selected =
        verifier.verify(
            prepared,
            selectedOriginalEdgeIds
        );

    if (!selected.valid) {
        result.message =
            "HOMKERNEL lookup edge set was not verified as a Kuratowski subdivision.";
        return result;
    }

    return selected;
}

std::vector<int> KuratowskiKernelSelector::expandSkeletonEdges(
    const std::vector<int>& selectedSkeletonEdgeIds,
    const HomKernelSignature& signature
) {
    std::vector<int> originalEdgeIds;

    for (int skeletonEdgeId : selectedSkeletonEdgeIds) {
        const HomKernelSkeletonEdge* skeletonEdge =
            findSkeletonEdgeById(
                signature,
                skeletonEdgeId
            );

        if (skeletonEdge == nullptr) {
            continue;
        }

        originalEdgeIds.insert(
            originalEdgeIds.end(),
            skeletonEdge->originalEdgeIds.begin(),
            skeletonEdge->originalEdgeIds.end()
        );
    }

    return originalEdgeIds;
}

const HomKernelSkeletonEdge* KuratowskiKernelSelector::findSkeletonEdgeById(
    const HomKernelSignature& signature,
    int skeletonEdgeId
) {
    for (const HomKernelSkeletonEdge& skeletonEdge : signature.skeletonEdges) {
        if (skeletonEdge.id == skeletonEdgeId) {
            return &skeletonEdge;
        }
    }

    return nullptr;
}

} // namespace ht