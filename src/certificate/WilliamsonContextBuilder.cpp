#include "ht/certificate/WilliamsonContextBuilder.hpp"

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

    const int cycleNode = nodeForDart(pathTree, failure.cycleRootDart);

    if (cycleNode == -1) {
        context.message =
            "Could not map failure.cycleRootDart to a base cycle PathTree node.";
        return context;
    }

    std::vector<int> leftCandidates =
        collectCandidateNodes(pathTree, failure.blockLeftSegments);

    std::vector<int> rightCandidates =
        collectCandidateNodes(pathTree, failure.blockRightSegments);

    if (leftCandidates.empty() || rightCandidates.empty()) {
        context.message =
            "Could not collect left/right Williamson segment candidates.";
        return context;
    }

    const std::vector<int> fDartCandidates =
        collectFDartCandidates(failure);

    for (int fDart : fDartCandidates) {
        const int fNode = nodeForDart(pathTree, fDart);

        if (fNode == -1) {
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
            return mapped;
        }
    }

    context.message =
        "Could not map failure to Williamson context (F, A, B).";
    return context;
}

int WilliamsonContextBuilder::nodeForDart(
    const PathTree& pathTree,
    int dartId
) {
    if (dartId < 0 || dartId >= static_cast<int>(pathTree.nodeByDefiningDart.size())) {
        return -1;
    }

    return pathTree.nodeByDefiningDart[dartId];
}

std::vector<int> WilliamsonContextBuilder::collectFDartCandidates(
    const StrongPlanarityFailure& failure
) {
    std::vector<int> candidates;

    // For BothSidesAttachAboveW0 failures, the actual Williamson F is often
    // the root emanating segment that closes/interlaces with the two block sides.
    // We test only the first recorded root emanating dart, then fall back to
    // cycleRootDart. This keeps the mapping targeted and avoids all-pairs logic.
    if (!failure.cycleRootEmanatingDarts.empty()) {
        candidates.push_back(failure.cycleRootEmanatingDarts.front());
    }

    if (failure.cycleRootDart != -1) {
        if (candidates.empty() || candidates.front() != failure.cycleRootDart) {
            candidates.push_back(failure.cycleRootDart);
        }
    }

    return candidates;
}

std::vector<int> WilliamsonContextBuilder::collectCandidateNodes(
    const PathTree& pathTree,
    const std::vector<int>& dartIds
) {
    std::vector<int> nodes;

    for (int dartId : dartIds) {
        const int nodeId = nodeForDart(pathTree, dartId);

        if (nodeId == -1) {
            continue;
        }

        nodes.push_back(nodeId);
    }

    return nodes;
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

        if (directlyLinkedEitherDirection(tester, fNode, candidateNode)) {
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
        context.message = "Invalid node id while creating Williamson context.";
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
        directlyLinkedEitherDirection(tester, context.fNode, context.aNode);

    context.bLinkedToF =
        directlyLinkedEitherDirection(tester, context.fNode, context.bNode);

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

} // namespace ht