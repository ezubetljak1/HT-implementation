#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/PathTreeQueries.hpp"

#include <sstream>

namespace ht {

WilliamsonContext WilliamsonContextBuilder::build(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const StrongPlanarityFailure& failure
) const {
    WilliamsonContext context;

    if (!failure.hasFailure()) {
        context.message = "No strong-planarity failure is available.";
        return context;
    }

    DirectLinkTester directLinkTester(prepared, pathTree, metadata);

    const int cycleNode =
        nodeForDart(
            pathTree,
            failure.cycleRootDart
        );

    if (cycleNode == -1) {
        context.message =
            "Could not map failure.cycleRootDart to a base cycle PathTree node.";
        return context;
    }

    std::vector<int> leftCandidates =
        collectCandidateNodes(
            pathTree,
            failure.blockLeftSegments
        );

    appendCandidateNodes(
        leftCandidates,
        pathTree,
        failure.stackTopLeftSegments
    );

    std::vector<int> rightCandidates =
        collectCandidateNodes(
            pathTree,
            failure.blockRightSegments
        );

    appendCandidateNodes(
        rightCandidates,
        pathTree,
        failure.stackTopRightSegments
    );

    if (leftCandidates.empty() || rightCandidates.empty()) {
        std::ostringstream out;

        out << "Could not collect left/right Williamson segment candidates. "
            << "leftCandidates=" << leftCandidates.size()
            << ", rightCandidates=" << rightCandidates.size()
            << ", blockLeftSegments=" << failure.blockLeftSegments.size()
            << ", blockRightSegments=" << failure.blockRightSegments.size()
            << ", stackTopLeftSegments=" << failure.stackTopLeftSegments.size()
            << ", stackTopRightSegments=" << failure.stackTopRightSegments.size();

        context.message = out.str();
        return context;
    }

    const std::vector<int> fDartCandidates =
        collectFDartCandidates(failure);

    std::vector<int> fNodeCandidates;

    for (int fDart : fDartCandidates) {
        addUnique(
            fNodeCandidates,
            nodeForDart(
                pathTree,
                fDart
            )
        );
    }

    // Relabeling can expose the Williamson F segment through the witness
    // segment lists rather than through one of the primary failure darts.
    // This still stays linear: we only scan the already collected witness
    // candidates, not arbitrary segment pairs.
    for (int nodeId : leftCandidates) {
        addUnique(
            fNodeCandidates,
            nodeId
        );
    }

    for (int nodeId : rightCandidates) {
        addUnique(
            fNodeCandidates,
            nodeId
        );
    }

    WilliamsonContext fallbackContext;

    for (int fNode : fNodeCandidates) {
        if (fNode == -1) {
            continue;
        }

        if (fNode == cycleNode) {
            continue;
        }

        const int aNode =
            findFirstNodeLinkedToF(
                directLinkTester,
                fNode,
                leftCandidates
            );

        const int bNode =
            findFirstNodeLinkedToF(
                directLinkTester,
                fNode,
                rightCandidates
            );

        if (aNode == -1 || bNode == -1) {
            continue;
        }

        WilliamsonContext mapped =
            makeContext(
                pathTree,
                directLinkTester,
                fNode,
                aNode,
                bNode,
                cycleNode
            );

        if (mapped.valid) {
            if (contextNodesAppearAroundCycle(
                    prepared,
                    pathTree,
                    cycleNode,
                    fNode,
                    aNode,
                    bNode
                )) {
                return mapped;
            }

            if (!fallbackContext.valid) {
                fallbackContext = mapped;
            }
        }
    }

    if (fallbackContext.valid) {
        fallbackContext.valid = false;
        fallbackContext.message =
            "Mapped a direct-link Williamson context, but F/A/B were not all present around CYCLE(e).";
        return fallbackContext;
    }

    context.message =
        "Could not map failure to Williamson context (F, A, B).";

    return context;
}

int WilliamsonContextBuilder::nodeForDart(
    const PathTree& pathTree,
    int dartId
) {
    if (dartId < 0
        || dartId >= static_cast<int>(pathTree.nodeByDefiningDart.size())) {
        return -1;
    }

    return pathTree.nodeByDefiningDart[dartId];
}

void WilliamsonContextBuilder::addUnique(
    std::vector<int>& values,
    int value
) {
    if (value == -1) {
        return;
    }

    for (int existing : values) {
        if (existing == value) {
            return;
        }
    }

    values.push_back(value);
}

std::vector<int> WilliamsonContextBuilder::collectFDartCandidates(
    const StrongPlanarityFailure& failure
) {
    std::vector<int> candidates;

    // Keep this list intentionally small. We want a constant-size set of
    // plausible F darts, not an all-pairs search over the failure witness.
    if (!failure.cycleRootEmanatingDarts.empty()) {
        addUnique(
            candidates,
            failure.cycleRootEmanatingDarts.front()
        );
    }

    addUnique(
        candidates,
        failure.cycleRootDart
    );

    addUnique(
        candidates,
        failure.closingBackDart
    );

    addUnique(
        candidates,
        failure.currentDart
    );

    addUnique(
        candidates,
        failure.rootTreeDart
    );

    return candidates;
}

std::vector<int> WilliamsonContextBuilder::collectCandidateNodes(
    const PathTree& pathTree,
    const std::vector<int>& dartIds
) {
    std::vector<int> nodes;

    appendCandidateNodes(
        nodes,
        pathTree,
        dartIds
    );

    return nodes;
}

void WilliamsonContextBuilder::appendCandidateNodes(
    std::vector<int>& nodes,
    const PathTree& pathTree,
    const std::vector<int>& dartIds
) {
    for (int dartId : dartIds) {
        const int nodeId =
            nodeForDart(
                pathTree,
                dartId
            );

        addUnique(
            nodes,
            nodeId
        );
    }
}

bool WilliamsonContextBuilder::directlyLinkedEitherDirection(
    const DirectLinkTester& tester,
    int firstNode,
    int secondNode
) {
    return tester.directlyLinkedToEarlierSegment(firstNode, secondNode)
        || tester.directlyLinkedToEarlierSegment(secondNode, firstNode);
}

int WilliamsonContextBuilder::findFirstNodeLinkedToF(
    const DirectLinkTester& tester,
    int fNode,
    const std::vector<int>& candidateNodes
) {
    for (int candidateNode : candidateNodes) {
        if (candidateNode == -1) {
            continue;
        }

        if (candidateNode == fNode)
            continue;

        if (directlyLinkedEitherDirection(
                tester,
                fNode,
                candidateNode
            )) {
            return candidateNode;
        }
    }

    return -1;
}

WilliamsonContext WilliamsonContextBuilder::makeContext(
    const PathTree& pathTree,
    const DirectLinkTester& tester,
    int fNode,
    int aNode,
    int bNode,
    int cycleNode
) {
    WilliamsonContext context;

    if (fNode < 0 || fNode >= static_cast<int>(pathTree.nodes.size())
        || aNode < 0 || aNode >= static_cast<int>(pathTree.nodes.size())
        || bNode < 0 || bNode >= static_cast<int>(pathTree.nodes.size())
        || cycleNode < 0 || cycleNode >= static_cast<int>(pathTree.nodes.size())) {
        context.message =
            "Invalid node id while creating Williamson context.";
        return context;
    }

    if (fNode == cycleNode) {
        context.message =
            "Invalid Williamson context: F node equals the base cycle node.";
        return context;
    }

    if (fNode == aNode || fNode == bNode || aNode == bNode) {
        context.message =
            "Invalid Williamson context: F, A and B must be distinct nodes.";
        return context;
    }

    context.fNode = fNode;
    context.aNode = aNode;
    context.bNode = bNode;
    context.cycleNode = cycleNode;

    context.fDart = pathTree.nodes[fNode].definingDart;
    context.aDart = pathTree.nodes[aNode].definingDart;
    context.bDart = pathTree.nodes[bNode].definingDart;
    context.cycleDart = pathTree.nodes[cycleNode].definingDart;

    context.parentNode = pathTree.nodes[fNode].parent;

    context.aLinkedToF =
        directlyLinkedEitherDirection(
            tester,
            context.fNode,
            context.aNode
        );

    context.bLinkedToF =
        directlyLinkedEitherDirection(
            tester,
            context.fNode,
            context.bNode
        );

    context.valid =
        context.aLinkedToF
        && context.bLinkedToF
        && context.fNode != -1
        && context.aNode != -1
        && context.bNode != -1
        && context.cycleNode != -1;

    if (context.valid) {
        context.message =
            "Mapped strong-planarity failure to Williamson context (F, A, B).";
    } else {
        context.message =
            "Williamson context mapping failed direct-link validation.";
    }

    return context;
}


bool WilliamsonContextBuilder::containsNode(
    const std::vector<int>& nodes,
    int nodeId
) {
    for (int value : nodes) {
        if (value == nodeId) {
            return true;
        }
    }

    return false;
}

std::vector<int> WilliamsonContextBuilder::collectSegmentNodesAroundCycle(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    int cycleNode
) {
    std::vector<int> segmentNodes;

    if (cycleNode < 0 || cycleNode >= static_cast<int>(pathTree.nodes.size())) {
        return segmentNodes;
    }

    PathTreeQueries queries(prepared, pathTree);

    std::vector<int> cycleDarts =
        queries.cycleDartsForNode(cycleNode);

    std::vector<char> dartIsOnBaseCycle(
        static_cast<std::size_t>(prepared.darts.size()),
        0
    );

    std::vector<char> seenCycleVertex(
        static_cast<std::size_t>(prepared.n),
        0
    );

    std::vector<int> cycleVertices;

    for (int dartId : cycleDarts) {
        if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
            continue;
        }

        dartIsOnBaseCycle[dartId] = 1;

        const int reverseDart = prepared.darts[dartId].rev;

        if (reverseDart >= 0
            && reverseDart < static_cast<int>(dartIsOnBaseCycle.size())) {
            dartIsOnBaseCycle[reverseDart] = 1;
        }

        const Dart& dart = prepared.darts[dartId];

        if (dart.from >= 0
            && dart.from < prepared.n
            && !seenCycleVertex[dart.from]) {
            seenCycleVertex[dart.from] = 1;
            cycleVertices.push_back(dart.from);
        }

        if (dart.to >= 0
            && dart.to < prepared.n
            && !seenCycleVertex[dart.to]) {
            seenCycleVertex[dart.to] = 1;
            cycleVertices.push_back(dart.to);
        }
    }

    std::vector<char> seenNode(
        static_cast<std::size_t>(pathTree.nodes.size()),
        0
    );

    for (int vertex : cycleVertices) {
        if (vertex < 0 || vertex >= prepared.n) {
            continue;
        }

        for (int dartId : prepared.orderedOut[vertex]) {
            if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
                continue;
            }

            if (dartIsOnBaseCycle[dartId]) {
                continue;
            }

            const int directNode =
                nodeForDart(
                    pathTree,
                    dartId
                );

            addUnique(
                segmentNodes,
                directNode
            );

            const int reverseDart =
                prepared.darts[dartId].rev;

            if (reverseDart < 0
                || reverseDart >= static_cast<int>(prepared.darts.size())) {
                continue;
            }

            if (dartIsOnBaseCycle[reverseDart]) {
                continue;
            }

            const int reverseNode =
                nodeForDart(
                    pathTree,
                    reverseDart
                );

            addUnique(
                segmentNodes,
                reverseNode
            );
        }
    }

    return segmentNodes;
}

bool WilliamsonContextBuilder::contextNodesAppearAroundCycle(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    int cycleNode,
    int fNode,
    int aNode,
    int bNode
) {
    std::vector<int> segmentNodes =
        collectSegmentNodesAroundCycle(
            prepared,
            pathTree,
            cycleNode
        );

    return containsNode(segmentNodes, fNode)
        && containsNode(segmentNodes, aNode)
        && containsNode(segmentNodes, bNode);
}

} // namespace ht