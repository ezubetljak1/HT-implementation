#include "ht/certificate/WilliamsonKernelBuilder.hpp"

namespace ht {

WilliamsonKernel WilliamsonKernelBuilder::buildDirectKernel(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const WilliamsonBasicCase& basicCase
) const {
    WilliamsonKernel kernel;
    kernel.basicCase = basicCase;

    if (!basicCase.valid) {
        kernel.message = "Cannot build Williamson kernel from invalid basic case.";
        return kernel;
    }

    if (basicCase.type != WilliamsonBasicCaseType::DirectTriangleLinks) {
        kernel.message =
            "Only Williamson direct triangle-link basic case is supported by this builder.";
        return kernel;
    }

    const int maxOriginalId = maxOriginalEdgeId(prepared);

    if (maxOriginalId < 0) {
        kernel.message = "Prepared graph contains no original edge IDs.";
        return kernel;
    }

    std::vector<char> seen(static_cast<std::size_t>(maxOriginalId + 1), 0);
    PathTreeQueries queries(prepared, pathTree);

    const WilliamsonContext& context = basicCase.context;

    // Basic case 1: F dl B dl A dl F.
    // Materialize only the selected constant-size set of segments.
    addKernelNodeContribution(
        prepared,
        queries,
        context.fNode,
        seen,
        kernel.originalEdgeIds
    );

    for (int nodeId : basicCase.segmentPathNodes) {
        addKernelNodeContribution(
            prepared,
            queries,
            nodeId,
            seen,
            kernel.originalEdgeIds
        );
    }

    // Include the base failure cycle CYCLE(e), where e = failure.cycleRootDart.
    // This is the Williamson base cycle, not necessarily the parent of F.
    if (context.cycleNode != -1) {
        addCycleForNode(
            prepared,
            queries,
            context.cycleNode,
            seen,
            kernel.originalEdgeIds
        );
    }

    kernel.valid = !kernel.originalEdgeIds.empty();

    if (kernel.valid) {
        kernel.message =
            "Built Williamson direct basic-case kernel candidate.";
    } else {
        kernel.message =
            "Williamson direct basic-case kernel produced no original edges.";
    }

    return kernel;
}

int WilliamsonKernelBuilder::maxOriginalEdgeId(
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

void WilliamsonKernelBuilder::addUniqueOriginalEdgeFromDart(
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

void WilliamsonKernelBuilder::addUniqueOriginalEdgesFromDarts(
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

void WilliamsonKernelBuilder::addCycleForNode(
    const PreparedPalmTree& prepared,
    const PathTreeQueries& queries,
    int nodeId,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
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

void WilliamsonKernelBuilder::addSegmentForNode(
    const PreparedPalmTree& prepared,
    const PathTreeQueries& queries,
    int nodeId,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
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
}

void WilliamsonKernelBuilder::addKernelNodeContribution(
    const PreparedPalmTree& prepared,
    const PathTreeQueries& queries,
    int nodeId,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
    if (nodeId == -1) {
        return;
    }

    addSegmentForNode(
        prepared,
        queries,
        nodeId,
        seen,
        originalEdgeIds
    );

    addCycleForNode(
        prepared,
        queries,
        nodeId,
        seen,
        originalEdgeIds
    );
}

} // namespace ht