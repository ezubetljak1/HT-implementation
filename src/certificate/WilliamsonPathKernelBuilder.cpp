#include "ht/certificate/WilliamsonPathKernelBuilder.hpp"

#include "ht/certificate/WilliamsonSecondCaseKernelBuilder.hpp"

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

WilliamsonKernel WilliamsonPathKernelBuilder::build(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonKernel kernel;
    kernel.context = context;
    kernel.segfoPath = segfoPath;

    const NormalizedPath normalized =
        normalizeToBasicCase(
            prepared,
            pathTree,
            metadata,
            fList,
            context,
            segfoPath
        );

    if (!normalized.valid) {
        kernel.message =
            "Could not normalize Williamson SEGFO path. " +
            normalized.message;
        return kernel;
    }

    if (normalized.isBasicCase1) {
        return buildBasicCase1(
            prepared,
            pathTree,
            metadata,
            fList,
            normalized.context,
            normalized.segfoPath
        );
    }

    if (normalized.isBasicCase2) {
        WilliamsonSecondCaseKernelBuilder secondCaseBuilder;

        return secondCaseBuilder.build(
            prepared,
            pathTree,
            metadata,
            fList,
            normalized.context,
            normalized.segfoPath
        );
    }

    kernel.message =
        "Williamson path normalized, but it is neither Basic Case 1 nor Basic Case 2.";

    return kernel;
}

WilliamsonKernel WilliamsonPathKernelBuilder::build(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonSegmentList segmentList;
    segmentList.valid = true;
    segmentList.message =
        "Compatibility SEGLIST built from SEGFO path and F node.";

    for (int nodeId : segfoPath.segmentPathNodes) {
        bool exists = false;

        for (int existing : segmentList.segmentNodes) {
            if (existing == nodeId) {
                exists = true;
                break;
            }
        }

        if (!exists && nodeId >= 0) {
            segmentList.segmentNodes.push_back(nodeId);
        }
    }

    bool hasF = false;

    for (int existing : segmentList.segmentNodes) {
        if (existing == context.fNode) {
            hasF = true;
            break;
        }
    }

    if (!hasF && context.fNode >= 0) {
        segmentList.segmentNodes.push_back(context.fNode);
    }

    WilliamsonFListBuilder fListBuilder;

    WilliamsonFList fList =
        fListBuilder.buildFromSegmentList(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context.fNode
        );

    if (!fList.valid) {
        WilliamsonKernel kernel;
        kernel.context = context;
        kernel.segfoPath = segfoPath;
        kernel.valid = false;
        kernel.message =
            "Compatibility WilliamsonPathKernelBuilder overload failed to build FLIST.";
        return kernel;
    }

    return build(
        prepared,
        pathTree,
        metadata,
        fList,
        context,
        segfoPath
    );
}

WilliamsonPathKernelBuilder::NormalizedPath
WilliamsonPathKernelBuilder::normalizeToBasicCase(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) {
    NormalizedPath result;

    if (!context.valid) {
        result.message = "Invalid Williamson context.";
        return result;
    }

    if (!segfoPath.valid || segfoPath.segmentPathNodes.size() < 2) {
        result.message = "Invalid SEGFO path.";
        return result;
    }

    WilliamsonContext currentContext = context;
    WilliamsonSegfoPath currentPath = segfoPath;

    bool changed = true;

    while (changed) {
        changed = false;

        const std::vector<int>& nodes = currentPath.segmentPathNodes;

        if (nodes.size() < 2) {
            result.message = "SEGFO path became too short during normalization.";
            return result;
        }

        currentContext.bNode = nodes.front();
        currentContext.aNode = nodes.back();

        currentContext.aLinkedToF =
            internalNodeLinksToF(
                prepared,
                pathTree,
                metadata,
                fList,
                currentContext.aNode,
                currentContext.fNode
            );

        currentContext.bLinkedToF =
            internalNodeLinksToF(
                prepared,
                pathTree,
                metadata,
                fList,
                currentContext.bNode,
                currentContext.fNode
            );

        if (!currentContext.aLinkedToF || !currentContext.bLinkedToF) {
            result.message =
                "Path endpoints are not both directly linked to F.";
            return result;
        }

        if (nodes.size() == 2) {
            const int firstNode = nodes[0];
            const int secondNode = nodes[1];

            WilliamsonLinkOracle oracle(
                prepared,
                metadata,
                fList
            );

            const bool secondLinksFirst =
                oracle.linksToOpenSpan(
                    secondNode,
                    firstNode
                );

            const bool firstLinksSecond =
                oracle.linksToOpenSpan(
                    firstNode,
                    secondNode
                );

            if (secondLinksFirst) {
                currentContext.bNode = firstNode;
                currentContext.aNode = secondNode;

                currentContext.bLinkedToF =
                    internalNodeLinksToF(
                        prepared,
                        pathTree,
                        metadata,
                        fList,
                        currentContext.bNode,
                        currentContext.fNode
                    );

                currentContext.aLinkedToF =
                    internalNodeLinksToF(
                        prepared,
                        pathTree,
                        metadata,
                        fList,
                        currentContext.aNode,
                        currentContext.fNode
                    );

                if (!currentContext.aLinkedToF || !currentContext.bLinkedToF) {
                    result.message =
                        "Two-node SEGFO path has A dl B, but endpoints do not both link F.";
                    return result;
                }

                currentPath.segmentPathNodes = {
                    currentContext.bNode,
                    currentContext.aNode
                };

                currentPath.valid = true;
                currentPath.message =
                    "Oriented two-node SEGFO path as B -> A for Basic Case 1.";

                result.valid = true;
                result.isBasicCase1 = true;
                result.context = currentContext;
                result.segfoPath = currentPath;
                result.message =
                    "Normalized to Williamson Basic Case 1 with A dl B.";
                return result;
            }

            if (firstLinksSecond) {
                currentContext.bNode = secondNode;
                currentContext.aNode = firstNode;

                currentContext.bLinkedToF =
                    internalNodeLinksToF(
                        prepared,
                        pathTree,
                        metadata,
                        fList,
                        currentContext.bNode,
                        currentContext.fNode
                    );

                currentContext.aLinkedToF =
                    internalNodeLinksToF(
                        prepared,
                        pathTree,
                        metadata,
                        fList,
                        currentContext.aNode,
                        currentContext.fNode
                    );

                if (!currentContext.aLinkedToF || !currentContext.bLinkedToF) {
                    result.message =
                        "Reversed two-node SEGFO path has A dl B, but endpoints do not both link F.";
                    return result;
                }

                currentPath.segmentPathNodes = {
                    currentContext.bNode,
                    currentContext.aNode
                };

                currentPath.valid = true;
                currentPath.message =
                    "Reversed two-node SEGFO path to B -> A for Basic Case 1.";

                result.valid = true;
                result.isBasicCase1 = true;
                result.context = currentContext;
                result.segfoPath = currentPath;
                result.message =
                    "Normalized reversed two-node SEGFO path to Williamson Basic Case 1.";
                return result;
            }

            result.message =
                "Two-node SEGFO path cannot be oriented as Basic Case 1: "
                "neither node directly links into the other's open span.";
            return result;
        }

        for (int i = 1; i < static_cast<int>(nodes.size()) - 1; ++i) {
            const int yiNode = nodes[i];

            const bool yiLinksF =
                internalNodeLinksToF(
                    prepared,
                    pathTree,
                    metadata,
                    fList,
                    yiNode,
                    currentContext.fNode
                );

            if (!yiLinksF) {
                continue;
            }

            if (i % 2 == 1) {
                currentPath =
                    makeReducedPath(
                        nodes,
                        0,
                        i
                    );

                currentContext =
                    makeReducedContext(
                        currentContext,
                        currentPath.segmentPathNodes.front(),
                        currentPath.segmentPathNodes.back()
                    );
            } else {
                currentPath =
                    makeReducedPath(
                        nodes,
                        i,
                        static_cast<int>(nodes.size()) - 1
                    );

                currentContext =
                    makeReducedContext(
                        currentContext,
                        currentPath.segmentPathNodes.front(),
                        currentPath.segmentPathNodes.back()
                    );
            }

            changed = true;
            break;
        }
    }

    if (isCleanBasicCase2(
            prepared,
            pathTree,
            metadata,
            fList,
            currentContext,
            currentPath
        )) {
        result.valid = true;
        result.isBasicCase2 = true;
        result.context = currentContext;
        result.segfoPath = currentPath;
        result.message =
            "Normalized to clean Williamson Basic Case 2.";
        return result;
    }

    result.message =
        "SEGFO path has no internal Yi linked to F, but does not satisfy clean Basic Case 2.";

    return result;
}

bool WilliamsonPathKernelBuilder::internalNodeLinksToF(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    int nodeId,
    int fNode
) {
    (void) pathTree;

    WilliamsonLinkOracle oracle(
        prepared,
        metadata,
        fList
    );

    return oracle.linksToOpenSpan(
        nodeId,
        fNode
    );
}

WilliamsonContext WilliamsonPathKernelBuilder::makeReducedContext(
    const WilliamsonContext& oldContext,
    int newBNode,
    int newANode
) {
    WilliamsonContext context = oldContext;

    context.bNode = newBNode;
    context.aNode = newANode;

    context.bDart = -1;
    context.aDart = -1;

    context.aLinkedToF = true;
    context.bLinkedToF = true;

    context.message =
        "Williamson context reduced by internal Yi directly linked to F.";

    return context;
}

WilliamsonSegfoPath WilliamsonPathKernelBuilder::makeReducedPath(
    const std::vector<int>& oldPath,
    int startIndex,
    int endIndex
) {
    WilliamsonSegfoPath path;

    if (startIndex < 0 ||
        endIndex < startIndex ||
        endIndex >= static_cast<int>(oldPath.size())) {
        path.valid = false;
        path.message = "Invalid reduced SEGFO path interval.";
        return path;
    }

    path.segmentPathNodes.assign(
        oldPath.begin() + startIndex,
        oldPath.begin() + endIndex + 1
    );

    path.valid = path.segmentPathNodes.size() >= 2;
    path.message = "Reduced SEGFO path after internal Yi linked to F.";

    return path;
}

bool WilliamsonPathKernelBuilder::isCleanBasicCase2(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) {
    if (!context.valid || !segfoPath.valid) {
        return false;
    }

    const std::vector<int>& nodes = segfoPath.segmentPathNodes;

    if (nodes.size() <= 2) {
        return false;
    }

    if (nodes.front() != context.bNode ||
        nodes.back() != context.aNode) {
        return false;
    }

    if (nodes.size() % 2 != 0) {
        return false;
    }

    if (!internalNodeLinksToF(
            prepared,
            pathTree,
            metadata,
            fList,
            context.bNode,
            context.fNode
        )) {
        return false;
    }

    if (!internalNodeLinksToF(
            prepared,
            pathTree,
            metadata,
            fList,
            context.aNode,
            context.fNode
        )) {
        return false;
    }

    for (int i = 1; i < static_cast<int>(nodes.size()) - 1; ++i) {
        if (internalNodeLinksToF(
                prepared,
                pathTree,
                metadata,
                fList,
                nodes[i],
                context.fNode
            )) {
            return false;
        }
    }

    return true;
}

WilliamsonKernel WilliamsonPathKernelBuilder::buildBasicCase1(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonKernel kernel;
    kernel.context = context;
    kernel.segfoPath = segfoPath;

    if (!context.valid) {
        kernel.message =
            "Cannot build Basic Case 1 kernel: invalid Williamson context.";
        return kernel;
    }

    if (!segfoPath.valid || segfoPath.segmentPathNodes.empty()) {
        kernel.message =
            "Cannot build Basic Case 1 kernel: invalid SEGFO path.";
        return kernel;
    }

    if (!isBasicCase1(
            prepared,
            pathTree,
            metadata,
            fList,
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
            fList,
            context.aNode,
            context.bNode,
            collector
        );

    const bool addedAToF =
        addDirectLinkPath(
            prepared,
            pathTree,
            metadata,
            fList,
            context.aNode,
            context.fNode,
            collector
        );

    const bool addedBToF =
        addDirectLinkPath(
            prepared,
            pathTree,
            metadata,
            fList,
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

WilliamsonKernel WilliamsonPathKernelBuilder::buildBasicCase1(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    return build(
        prepared,
        pathTree,
        metadata,
        context,
        segfoPath
    );
}

bool WilliamsonPathKernelBuilder::isBasicCase1(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) {
    (void) pathTree;

    if (!context.valid || !segfoPath.valid) {
        return false;
    }

    if (segfoPath.segmentPathNodes.size() != 2) {
        return false;
    }

    if (segfoPath.segmentPathNodes[0] != context.bNode ||
        segfoPath.segmentPathNodes[1] != context.aNode) {
        return false;
    }

    WilliamsonLinkOracle oracle(
        prepared,
        metadata,
        fList
    );

    const bool aToB =
        oracle.linksToOpenSpan(
            context.aNode,
            context.bNode
        );

    const bool aToF =
        oracle.linksToOpenSpan(
            context.aNode,
            context.fNode
        );

    const bool bToF =
        oracle.linksToOpenSpan(
            context.bNode,
            context.fNode
        );

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
    const WilliamsonFList& fList,
    int sourceNode,
    int spanNode,
    EdgeCollector& out
) {
    WilliamsonLinkOracle oracle(
        prepared,
        metadata,
        fList
    );

    const WilliamsonLinkWitness witness =
        oracle.findLinkToOpenSpan(
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