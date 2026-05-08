#include "ht/certificate/WilliamsonPathKernelBuilder.hpp"

#include <stdexcept>

namespace ht {

WilliamsonPathKernelBuilder::EdgeCollector::EdgeCollector(
    const PreparedPalmTree& prepared
) : prepared(prepared) {
    const int maxId = WilliamsonPathKernelBuilder::maxOriginalEdgeId(prepared);

    if (maxId >= 0) {
        seenOriginalEdge.assign(
            static_cast<std::size_t>(maxId + 1),
            0
        );
    }
}

void WilliamsonPathKernelBuilder::EdgeCollector::addDart(int dartId) {
    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        return;
    }

    const int originalEdgeId = prepared.darts[dartId].originalEdgeId;

    if (originalEdgeId < 0 ||
        originalEdgeId >= static_cast<int>(seenOriginalEdge.size())) {
        return;
    }

    if (seenOriginalEdge[originalEdgeId]) {
        return;
    }

    seenOriginalEdge[originalEdgeId] = 1;
    originalEdgeIds.push_back(originalEdgeId);
}

void WilliamsonPathKernelBuilder::EdgeCollector::addDarts(
    const std::vector<int>& dartIds
) {
    for (int dartId : dartIds) {
        addDart(dartId);
    }
}

int WilliamsonPathKernelBuilder::maxOriginalEdgeId(
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

const SegmentMetadata& WilliamsonPathKernelBuilder::segmentMetadata(
    const SegmentMetadataTable& metadata,
    int nodeId
) {
    if (nodeId < 0 ||
        nodeId >= static_cast<int>(metadata.segmentByNode.size())) {
        throw std::runtime_error(
            "Invalid node id in WilliamsonPathKernelBuilder."
        );
    }

    const int segmentId = metadata.segmentByNode[nodeId];

    if (segmentId < 0 ||
        segmentId >= static_cast<int>(metadata.segments.size())) {
        throw std::runtime_error(
            "Invalid segment id in WilliamsonPathKernelBuilder."
        );
    }

    return metadata.segments[segmentId];
}

WilliamsonKernel WilliamsonPathKernelBuilder::buildBasicCase1(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonKernel kernel;
    kernel.context = context;
    kernel.segfoPath = segfoPath;

    if (!context.valid) {
        kernel.message = "Cannot build Basic Case 1 kernel: invalid Williamson context.";
        return kernel;
    }

    if (!segfoPath.valid || segfoPath.segmentPathNodes.empty()) {
        kernel.message = "Cannot build Basic Case 1 kernel: invalid SEGFO path.";
        return kernel;
    }

    if (!isBasicCase1(
            prepared,
            pathTree,
            metadata,
            context,
            segfoPath
        )) {
        kernel.message =
            "Not Williamson Basic Case 1. "
            "Expected SEGFO path B -> A with A dl B, A dl F, and B dl F.";
        return kernel;
    }

    EdgeCollector collector(prepared);
    PathTreeQueries queries(prepared, pathTree);

    // Williamson Basic Case 1 kernel:
    //
    //     CYCLE(e)
    //   + REDSEG(A)
    //   + REDSEG(B)
    //   + REDSEG(F)
    //   + direct-link path A -> B
    //   + direct-link path A -> F
    //   + direct-link path B -> F
    //
    // Important: we do NOT add whole SEG(A), SEG(B), or SEG(F).
    collector.addDarts(
        queries.cycleDartsForNode(context.cycleNode)
    );

    const bool addedRedA =
        addReducedSegment(
            prepared,
            pathTree,
            metadata,
            context.aNode,
            collector
        );

    const bool addedRedB =
        addReducedSegment(
            prepared,
            pathTree,
            metadata,
            context.bNode,
            collector
        );

    const bool addedRedF =
        addReducedSegment(
            prepared,
            pathTree,
            metadata,
            context.fNode,
            collector
        );

    const bool addedAToB =
        addDirectLinkPath(
            prepared,
            pathTree,
            metadata,
            context.aNode,
            context.bNode,
            collector
        );

    const bool addedAToF =
        addDirectLinkPath(
            prepared,
            pathTree,
            metadata,
            context.aNode,
            context.fNode,
            collector
        );

    const bool addedBToF =
        addDirectLinkPath(
            prepared,
            pathTree,
            metadata,
            context.bNode,
            context.fNode,
            collector
        );

    if (!(addedRedA &&
          addedRedB &&
          addedRedF &&
          addedAToB &&
          addedAToF &&
          addedBToF)) {
        kernel.message =
            "Williamson Basic Case 1 shape was detected, but one or more "
            "required witness paths could not be materialized.";
        return kernel;
    }

    kernel.originalEdgeIds = collector.originalEdgeIds;
    kernel.valid = !kernel.originalEdgeIds.empty();

    if (kernel.valid) {
        kernel.message =
            "Built Williamson Basic Case 1 kernel from CYCLE(e), "
            "REDSEG(A/B/F), and direct-link witness paths.";
    } else {
        kernel.message =
            "Williamson Basic Case 1 kernel construction produced no edges.";
    }

    return kernel;
}

bool WilliamsonPathKernelBuilder::isBasicCase1(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) {
    if (!context.valid || !segfoPath.valid) {
        return false;
    }

    // Basic Case 1:
    //
    //     F dl B dl A dl F
    //
    // In the current path representation this means the SEGFO path from
    // B to A has exactly two nodes: B, A.
    if (segfoPath.segmentPathNodes.size() != 2) {
        return false;
    }

    if (segfoPath.segmentPathNodes[0] != context.bNode ||
        segfoPath.segmentPathNodes[1] != context.aNode) {
        return false;
    }

    DirectLinkTester direct(
        prepared,
        pathTree,
        metadata
    );

    const bool aToB =
        direct.findPathFromSegmentToOpenSpan(
            context.aNode,
            context.bNode
        ).exists;

    const bool aToF =
        direct.findPathFromSegmentToOpenSpan(
            context.aNode,
            context.fNode
        ).exists;

    const bool bToF =
        direct.findPathFromSegmentToOpenSpan(
            context.bNode,
            context.fNode
        ).exists;

    return aToB && aToF && bToF;
}

bool WilliamsonPathKernelBuilder::addReducedSegment(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    int nodeId,
    EdgeCollector& out
) {
    const SegmentMetadata& segment =
        segmentMetadata(metadata, nodeId);

    bool addedAny = false;

    if (segment.low1Witness.backDart != -1) {
        addedAny |= addLowWitnessPath(
            prepared,
            pathTree,
            segment,
            segment.low1Witness,
            out
        );
    }

    if (segment.low2Witness.backDart != -1) {
        addedAny |= addLowWitnessPath(
            prepared,
            pathTree,
            segment,
            segment.low2Witness,
            out
        );
    }

    return addedAny;
}

bool WilliamsonPathKernelBuilder::addLowWitnessPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadata& segment,
    const SegmentHeadWitness& witness,
    EdgeCollector& out
) {
    if (segment.tailVertex < 0 ||
        witness.backTailVertex < 0 ||
        witness.backDart < 0) {
        return false;
    }

    if (witness.backDart >= static_cast<int>(prepared.darts.size())) {
        return false;
    }

    PathTreeQueries queries(prepared, pathTree);

    std::vector<int> treePath =
        queries.treePathDarts(
            segment.tailVertex,
            witness.backTailVertex
        );

    out.addDarts(treePath);
    out.addDart(witness.backDart);

    return true;
}

bool WilliamsonPathKernelBuilder::addDirectLinkPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    int sourceNode,
    int spanNode,
    EdgeCollector& out
) {
    DirectLinkTester direct(
        prepared,
        pathTree,
        metadata
    );

    const DirectLinkWitness witness =
        direct.findPathFromSegmentToOpenSpan(
            sourceNode,
            spanNode
        );

    if (!witness.exists ||
        witness.backDart < 0 ||
        witness.backTailVertex < 0) {
        return false;
    }

    const SegmentMetadata& sourceSegment =
        segmentMetadata(metadata, sourceNode);

    if (sourceSegment.tailVertex < 0) {
        return false;
    }

    PathTreeQueries queries(prepared, pathTree);

    std::vector<int> treePath =
        queries.treePathDarts(
            sourceSegment.tailVertex,
            witness.backTailVertex
        );

    out.addDarts(treePath);
    out.addDart(witness.backDart);

    return true;
}

} // namespace ht