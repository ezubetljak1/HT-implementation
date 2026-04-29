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

    const int fNode = nodeForDart(pathTree, failure.cycleRootDart);

    if (fNode == -1) {
        context.message = "Could not map failure.cycleRootDart to a PathTree node.";
        return context;
    }

    std::vector<int> leftCandidates =
        collectCandidateNodes(pathTree, failure.blockLeftSegments);

    std::vector<int> rightCandidates =
        collectCandidateNodes(pathTree, failure.blockRightSegments);

    const int aNode =
        findFirstNodeLinkedToF(directLinkTester, fNode, leftCandidates);

    const int bNode =
        findFirstNodeLinkedToF(directLinkTester, fNode, rightCandidates);

    if (aNode == -1 || bNode == -1) {
        context.message =
            "Could not find both A and B segments directly linked to F.";
        return context;
    }

    context.fNode = fNode;
    context.aNode = aNode;
    context.bNode = bNode;

    context.fDart = pathTree.nodes[fNode].definingDart;
    context.aDart = pathTree.nodes[aNode].definingDart;
    context.bDart = pathTree.nodes[bNode].definingDart;

    context.parentNode = pathTree.nodes[fNode].parent;

    context.aLinkedToF =
        directlyLinkedEitherDirection(directLinkTester, fNode, aNode);

    context.bLinkedToF =
        directlyLinkedEitherDirection(directLinkTester, fNode, bNode);

    context.valid =
        context.aLinkedToF
        && context.bLinkedToF
        && context.fNode != -1
        && context.aNode != -1
        && context.bNode != -1;

    if (context.valid) {
        context.message =
            "Mapped strong-planarity failure to Williamson context (F, A, B).";
    } else {
        context.message =
            "Williamson context mapping failed direct-link validation.";
    }

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

} // namespace ht