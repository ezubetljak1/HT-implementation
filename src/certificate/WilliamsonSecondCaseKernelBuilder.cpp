#include "ht/certificate/WilliamsonSecondCaseKernelBuilder.hpp"

#include <stdexcept>

namespace ht {

WilliamsonSecondCaseKernelBuilder::EdgeCollector::EdgeCollector(
    const PreparedPalmTree& prepared
) : prepared(prepared) {
    const int maxId =
        WilliamsonSecondCaseKernelBuilder::maxOriginalEdgeId(prepared);

    if (maxId >= 0) {
        seenOriginalEdge.assign(
            static_cast<std::size_t>(maxId + 1),
            0
        );
    }
}

void WilliamsonSecondCaseKernelBuilder::EdgeCollector::addDart(int dartId) {
    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        return;
    }

    const int originalEdgeId =
        prepared.darts[dartId].originalEdgeId;

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

void WilliamsonSecondCaseKernelBuilder::EdgeCollector::addDarts(
    const std::vector<int>& dartIds
) {
    for (int dartId : dartIds) {
        addDart(dartId);
    }
}

int WilliamsonSecondCaseKernelBuilder::maxOriginalEdgeId(
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

const SegmentMetadata& WilliamsonSecondCaseKernelBuilder::segmentMetadata(
    const SegmentMetadataTable& metadata,
    int nodeId
) {
    if (nodeId < 0 ||
        nodeId >= static_cast<int>(metadata.segmentByNode.size())) {
        throw std::runtime_error(
            "Invalid node id in WilliamsonSecondCaseKernelBuilder."
        );
    }

    const int segmentId = metadata.segmentByNode[nodeId];

    if (segmentId < 0 ||
        segmentId >= static_cast<int>(metadata.segments.size())) {
        throw std::runtime_error(
            "Invalid segment id in WilliamsonSecondCaseKernelBuilder."
        );
    }

    return metadata.segments[segmentId];
}

WilliamsonKernel WilliamsonSecondCaseKernelBuilder::build(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonKernel kernel;
    kernel.context = context;
    kernel.segfoPath = segfoPath;

    if (!isCleanSecondCaseShape(
            prepared,
            pathTree,
            metadata,
            context,
            segfoPath
        )) {
        kernel.message =
            "Cannot build Williamson second-case kernel: input is not a clean "
            "second basic case.";
        return kernel;
    }

    const std::vector<int>& nodes =
        segfoPath.segmentPathNodes;

    EdgeCollector collector(prepared);
    PathTreeQueries queries(prepared, pathTree);

    // CYCLE(e)
    collector.addDarts(
        queries.cycleDartsForNode(context.cycleNode)
    );

    // REDSEG(F)
    if (!addReducedSegment(
            prepared,
            pathTree,
            metadata,
            context.fNode,
            collector
        )) {
        kernel.message =
            "Second-case kernel failed: could not materialize REDSEG(F).";
        return kernel;
    }

    // REDSEG(B), REDSEG(Y1), ..., REDSEG(Yp), REDSEG(A)
    for (int nodeId : nodes) {
        if (!addReducedSegment(
                prepared,
                pathTree,
                metadata,
                nodeId,
                collector
            )) {
            kernel.message =
                "Second-case kernel failed: could not materialize REDSEG "
                "for a SEGFO path node.";
            return kernel;
        }
    }

    // Direct-link paths along:
    //
    //     B <- Y1 <- Y2 <- ... <- Yp <- A
    //
    // In this representation, each later node has a HEAD inside OSPAN(previous).
    for (int i = 0; i + 1 < static_cast<int>(nodes.size()); ++i) {
        const int previousNode = nodes[i];
        const int laterNode = nodes[i + 1];

        if (!addDirectLinkPath(
                prepared,
                pathTree,
                metadata,
                laterNode,
                previousNode,
                collector
            )) {
            kernel.message =
                "Second-case kernel failed: could not materialize direct-link "
                "path between consecutive SEGFO nodes.";
            return kernel;
        }
    }

    // Endpoint links:
    //
    //     B -> F
    //     A -> F
    //
    // These are part of F dl B ... A dl F.
    if (!addDirectLinkPath(
            prepared,
            pathTree,
            metadata,
            context.bNode,
            context.fNode,
            collector
        )) {
        kernel.message =
            "Second-case kernel failed: could not materialize B -> F link.";
        return kernel;
    }

    if (!addDirectLinkPath(
            prepared,
            pathTree,
            metadata,
            context.aNode,
            context.fNode,
            collector
        )) {
        kernel.message =
            "Second-case kernel failed: could not materialize A -> F link.";
        return kernel;
    }

    kernel.originalEdgeIds = collector.originalEdgeIds;
    kernel.valid = !kernel.originalEdgeIds.empty();

    if (kernel.valid) {
        kernel.message =
            "Built Williamson second-case path kernel from CYCLE(e), "
            "REDSEG(F), REDSEG(B/Y/A), consecutive SEGFO direct-link paths, "
            "and endpoint links to F.";
    } else {
        kernel.message =
            "Second-case kernel construction produced no edges.";
    }

    return kernel;
}

bool WilliamsonSecondCaseKernelBuilder::isCleanSecondCaseShape(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) {
    if (!context.valid || !segfoPath.valid) {
        return false;
    }

    const std::vector<int>& nodes =
        segfoPath.segmentPathNodes;

    if (nodes.size() <= 2) {
        return false;
    }

    // nodes = B, Y1, ..., Yp, A
    if (nodes.front() != context.bNode ||
        nodes.back() != context.aNode) {
        return false;
    }

    // p even and p > 0.
    //
    // nodes.size() = p + 2
    // p even <=> nodes.size() even.
    if (nodes.size() % 2 != 0) {
        return false;
    }

    if (!segmentLinksToSpan(
            prepared,
            pathTree,
            metadata,
            context.bNode,
            context.fNode
        )) {
        return false;
    }

    if (!segmentLinksToSpan(
            prepared,
            pathTree,
            metadata,
            context.aNode,
            context.fNode
        )) {
        return false;
    }

    // Internal Yi must NOT directly link F.
    for (int i = 1; i < static_cast<int>(nodes.size()) - 1; ++i) {
        if (segmentLinksToSpan(
                prepared,
                pathTree,
                metadata,
                nodes[i],
                context.fNode
            )) {
            return false;
        }
    }

    // Consecutive SEGFO links must exist:
    //
    //     Y1 -> B,
    //     Y2 -> Y1,
    //     ...
    //     A  -> Yp.
    for (int i = 0; i + 1 < static_cast<int>(nodes.size()); ++i) {
        if (!segmentLinksToSpan(
                prepared,
                pathTree,
                metadata,
                nodes[i + 1],
                nodes[i]
            )) {
            return false;
        }
    }

    return true;
}

bool WilliamsonSecondCaseKernelBuilder::segmentLinksToSpan(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    int sourceNode,
    int spanNode
) {
    DirectLinkTester direct(
        prepared,
        pathTree,
        metadata
    );

    return direct.findPathFromSegmentToOpenSpan(
        sourceNode,
        spanNode
    ).exists;
}

bool WilliamsonSecondCaseKernelBuilder::addReducedSegment(
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

bool WilliamsonSecondCaseKernelBuilder::addLowWitnessPath(
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

bool WilliamsonSecondCaseKernelBuilder::addDirectLinkPath(
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