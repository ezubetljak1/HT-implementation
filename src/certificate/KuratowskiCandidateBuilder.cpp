#include "ht/certificate/KuratowskiCandidateBuilder.hpp"

#include "ht/certificate/PathTreeBuilder.hpp"

namespace ht {

std::vector<int> KuratowskiCandidateBuilder::buildOriginalEdgeCandidate(
    const PreparedPalmTree& prepared,
    const StrongPlanarityFailure& failure
) const {
    std::vector<int> originalEdgeIds;

    const int maxOriginalId = maxOriginalEdgeId(prepared);

    if (maxOriginalId < 0) {
        return originalEdgeIds;
    }

    std::vector<char> seen(static_cast<std::size_t>(maxOriginalId + 1), 0);

    PathTreeBuilder pathTreeBuilder;
    PathTree pathTree = pathTreeBuilder.build(prepared);
    PathTreeQueries queries(prepared, pathTree);

    // Base failure/cycle context.
    addUniqueOriginalEdgeFromDart(
        prepared,
        failure.rootTreeDart,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgeFromDart(
        prepared,
        failure.cycleRootDart,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgeFromDart(
        prepared,
        failure.currentDart,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgeFromDart(
        prepared,
        failure.closingBackDart,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgesFromDarts(
        prepared,
        failure.cycleTreeDarts,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgesFromDarts(
        prepared,
        failure.cycleEmanatingDarts,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgesFromDarts(
        prepared,
        failure.cycleRootEmanatingDarts,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgesFromDarts(
        prepared,
        failure.blockLeftSegments,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgesFromDarts(
        prepared,
        failure.blockRightSegments,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgesFromDarts(
        prepared,
        failure.stackTopLeftSegments,
        seen,
        originalEdgeIds
    );

    addUniqueOriginalEdgesFromDarts(
        prepared,
        failure.stackTopRightSegments,
        seen,
        originalEdgeIds
    );

    // Targeted Williamson-style expansion.
    // Do not do this for all path-tree nodes. Only expand the constant-size witness.
    addCycleForDart(
        prepared,
        queries,
        failure.cycleRootDart,
        seen,
        originalEdgeIds
    );

    addSegmentsForDarts(
        prepared,
        queries,
        failure.blockLeftSegments,
        seen,
        originalEdgeIds
    );

    addSegmentsForDarts(
        prepared,
        queries,
        failure.blockRightSegments,
        seen,
        originalEdgeIds
    );

    addSegmentsForDarts(
        prepared,
        queries,
        failure.stackTopLeftSegments,
        seen,
        originalEdgeIds
    );

    addSegmentsForDarts(
        prepared,
        queries,
        failure.stackTopRightSegments,
        seen,
        originalEdgeIds
    );

    addSegmentsForDarts(
        prepared,
        queries,
        failure.cycleEmanatingDarts,
        seen,
        originalEdgeIds
    );

    addSegmentsForDarts(
        prepared,
        queries,
        failure.cycleRootEmanatingDarts,
        seen,
        originalEdgeIds
    );

    return originalEdgeIds;
}

int KuratowskiCandidateBuilder::maxOriginalEdgeId(
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

void KuratowskiCandidateBuilder::addUniqueOriginalEdgeFromDart(
    const PreparedPalmTree& prepared,
    int dartId,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        return;
    }

    const int originalEdgeId = prepared.darts[dartId].originalEdgeId;

    if (originalEdgeId < 0 || originalEdgeId >= static_cast<int>(seen.size())) {
        return;
    }

    if (seen[originalEdgeId]) {
        return;
    }

    seen[originalEdgeId] = 1;
    originalEdgeIds.push_back(originalEdgeId);
}

void KuratowskiCandidateBuilder::addUniqueOriginalEdgesFromDarts(
    const PreparedPalmTree& prepared,
    const std::vector<int>& dartIds,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
    for (int dartId : dartIds) {
        addUniqueOriginalEdgeFromDart(
            prepared,
            dartId,
            seen,
            originalEdgeIds
        );
    }
}

void KuratowskiCandidateBuilder::addCycleForDart(
    const PreparedPalmTree& prepared,
    const PathTreeQueries& queries,
    int dartId,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
    const int nodeId = queries.nodeForDefiningDart(dartId);

    if (nodeId == -1) {
        return;
    }

    const std::vector<int> cycleDarts =
        queries.cycleDartsForNode(nodeId);

    addUniqueOriginalEdgesFromDarts(
        prepared,
        cycleDarts,
        seen,
        originalEdgeIds
    );
}

void KuratowskiCandidateBuilder::addSegmentForDart(
    const PreparedPalmTree& prepared,
    const PathTreeQueries& queries,
    int dartId,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
    const int nodeId = queries.nodeForDefiningDart(dartId);

    if (nodeId == -1) {
        return;
    }

    const std::vector<int> segmentDarts =
        queries.segmentDefiningDartsForNode(nodeId);

    addUniqueOriginalEdgesFromDarts(
        prepared,
        segmentDarts,
        seen,
        originalEdgeIds
    );

    // Also include the cycle of the top selected segment.
    // This is targeted to the witness dart, not applied to every descendant.
    addCycleForDart(
        prepared,
        queries,
        dartId,
        seen,
        originalEdgeIds
    );
}

void KuratowskiCandidateBuilder::addSegmentsForDarts(
    const PreparedPalmTree& prepared,
    const PathTreeQueries& queries,
    const std::vector<int>& dartIds,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
    for (int dartId : dartIds) {
        addSegmentForDart(
            prepared,
            queries,
            dartId,
            seen,
            originalEdgeIds
        );
    }
}

} // namespace ht