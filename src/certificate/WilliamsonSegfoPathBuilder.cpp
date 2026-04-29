#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"

namespace ht {

WilliamsonSegfoPath WilliamsonSegfoPathBuilder::buildPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonSegmentList& segmentList,
    const WilliamsonContext& context
) const {
    WilliamsonSegfoPath result;

    if (!context.valid) {
        result.message = "Cannot build SEGFO path from invalid Williamson context.";
        return result;
    }

    if (!segmentList.valid) {
        result.message = "Cannot build SEGFO path from invalid SEGLIST(e).";
        return result;
    }

    DirectLinkTester directLinkTester(prepared, pathTree, metadata);

    // Basic case 1 / direct SEGFO edge:
    // B dl A.
    if (directlyLinkedEitherDirection(
            directLinkTester,
            context.bNode,
            context.aNode
        )) {
        result.valid = true;
        result.segmentPathNodes.push_back(context.bNode);
        result.segmentPathNodes.push_back(context.aNode);
        result.message = "Built direct SEGFO path: B -> A.";
        return result;
    }

    // First non-direct extension:
    // find one Y in SEGLIST(e) such that B dl Y and Y dl A.
    //
    // This is a single linear scan over SEGLIST(e), not all-pairs SEGGR construction.
    for (int yNode : segmentList.segmentNodes) {
        if (isContextNode(context, yNode)) {
            continue;
        }

        const bool bLinkedToY =
            directlyLinkedEitherDirection(
                directLinkTester,
                context.bNode,
                yNode
            );

        if (!bLinkedToY) {
            continue;
        }

        const bool yLinkedToA =
            directlyLinkedEitherDirection(
                directLinkTester,
                yNode,
                context.aNode
            );

        if (!yLinkedToA) {
            continue;
        }

        result.valid = true;
        result.segmentPathNodes.push_back(context.bNode);
        result.segmentPathNodes.push_back(yNode);
        result.segmentPathNodes.push_back(context.aNode);
        result.message = "Built one-intermediate SEGFO path: B -> Y -> A.";
        return result;
    }

    result.message =
        "No direct or one-intermediate SEGFO path found. "
        "General SEGFO path construction is the next Williamson step.";

    return result;
}

bool WilliamsonSegfoPathBuilder::directlyLinkedEitherDirection(
    const DirectLinkTester& tester,
    int firstNode,
    int secondNode
) {
    return tester.directlyLinkedToEarlierSegment(firstNode, secondNode)
        || tester.directlyLinkedToEarlierSegment(secondNode, firstNode);
}

bool WilliamsonSegfoPathBuilder::isContextNode(
    const WilliamsonContext& context,
    int nodeId
) {
    return nodeId == context.fNode
        || nodeId == context.aNode
        || nodeId == context.bNode
        || nodeId == context.cycleNode;
}

} // namespace ht