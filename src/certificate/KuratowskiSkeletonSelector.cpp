#include "ht/certificate/KuratowskiSkeletonSelector.hpp"

#include <sstream>

namespace ht {

KuratowskiSubdivisionVerification KuratowskiSkeletonSelector::select(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds
) const {
    KuratowskiSubdivisionVerification result;

    const std::vector<int> deduplicatedCandidate =
        deduplicateOriginalEdgeIds(
            prepared,
            candidateOriginalEdgeIds
        );

    if (deduplicatedCandidate.empty()) {
        result.message =
            "Bounded Williamson selector received an empty candidate edge set.";
        return result;
    }

    KuratowskiSubdivisionVerifier verifier;

    const KuratowskiSubdivisionVerification direct =
        verifier.verify(
            prepared,
            deduplicatedCandidate
        );

    if (direct.valid) {
        return direct;
    }

    constexpr int kMaxBoundedKernelEdges = 18;

    if (static_cast<int>(deduplicatedCandidate.size()) >
        kMaxBoundedKernelEdges) {
        std::ostringstream out;

        out << "Bounded Williamson selector refused unbounded search: "
            << "candidate has "
            << deduplicatedCandidate.size()
            << " original edges, cap is "
            << kMaxBoundedKernelEdges
            << ". This means the Williamson kernel must be reduced further "
            << "before final certificate extraction. Direct verifier said: "
            << direct.message;

        result.message = out.str();
        return result;
    }

    if (tryBoundedOriginalEdgeSubsets(
            prepared,
            deduplicatedCandidate,
            result
        )) {
        return result;
    }

    std::ostringstream out;

    out << "Bounded Williamson selector did not find a K5 or K3,3 "
        << "subdivision inside the "
        << deduplicatedCandidate.size()
        << "-edge kernel candidate. Direct verifier said: "
        << direct.message;

    result.message = out.str();
    return result;
}

std::vector<int> KuratowskiSkeletonSelector::deduplicateOriginalEdgeIds(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds
) {
    const int maxId =
        maxOriginalEdgeId(prepared);

    if (maxId < 0) {
        return {};
    }

    std::vector<char> seen(
        static_cast<std::size_t>(maxId + 1),
        0
    );

    std::vector<int> deduplicated;

    for (int originalEdgeId : candidateOriginalEdgeIds) {
        if (originalEdgeId < 0 ||
            originalEdgeId >= static_cast<int>(seen.size())) {
            continue;
        }

        if (seen[originalEdgeId]) {
            continue;
        }

        seen[originalEdgeId] = 1;
        deduplicated.push_back(originalEdgeId);
    }

    return deduplicated;
}

int KuratowskiSkeletonSelector::maxOriginalEdgeId(
    const PreparedPalmTree& prepared
) {
    int maxId = -1;

    for (const Dart& dart : prepared.darts) {
        if (dart.originalEdgeId > maxId) {
            maxId = dart.originalEdgeId;
        }
    }

    return maxId;
}

bool KuratowskiSkeletonSelector::tryBoundedOriginalEdgeSubsets(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds,
    KuratowskiSubdivisionVerification& result
) {
    const int candidateSize =
        static_cast<int>(candidateOriginalEdgeIds.size());

    // A Kuratowski subdivision has at least:
    // - 9 edges for K3,3
    // - 10 edges for K5
    //
    // With subdivisions, it may have more original edges, so we try all
    // subset sizes from 9 up to the bounded kernel size.
    for (int subsetSize = 9;
         subsetSize <= candidateSize;
         ++subsetSize) {
        std::vector<int> selectedOriginalEdgeIds;

        if (backtrackOriginalEdgeSubsets(
                prepared,
                candidateOriginalEdgeIds,
                subsetSize,
                0,
                selectedOriginalEdgeIds,
                result
            )) {
            return true;
        }
    }

    return false;
}

bool KuratowskiSkeletonSelector::backtrackOriginalEdgeSubsets(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds,
    int subsetSize,
    int startIndex,
    std::vector<int>& selectedOriginalEdgeIds,
    KuratowskiSubdivisionVerification& result
) {
    if (static_cast<int>(selectedOriginalEdgeIds.size()) == subsetSize) {
        KuratowskiSubdivisionVerifier verifier;

        const KuratowskiSubdivisionVerification verification =
            verifier.verify(
                prepared,
                selectedOriginalEdgeIds
            );

        if (verification.valid) {
            result = verification;
            return true;
        }

        return false;
    }

    const int remainingToChoose =
        subsetSize - static_cast<int>(selectedOriginalEdgeIds.size());

    const int candidateSize =
        static_cast<int>(candidateOriginalEdgeIds.size());

    for (int i = startIndex;
         i <= candidateSize - remainingToChoose;
         ++i) {
        selectedOriginalEdgeIds.push_back(
            candidateOriginalEdgeIds[i]
        );

        if (backtrackOriginalEdgeSubsets(
                prepared,
                candidateOriginalEdgeIds,
                subsetSize,
                i + 1,
                selectedOriginalEdgeIds,
                result
            )) {
            return true;
        }

        selectedOriginalEdgeIds.pop_back();
    }

    return false;
}

} // namespace ht