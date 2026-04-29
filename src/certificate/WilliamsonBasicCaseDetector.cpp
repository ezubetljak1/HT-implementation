#include "ht/certificate/WilliamsonBasicCaseDetector.hpp"

namespace ht {

WilliamsonBasicCase WilliamsonBasicCaseDetector::detect(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context
) const {
    WilliamsonBasicCase result;
    result.context = context;

    if (!context.valid) {
        result.message =
            "Cannot detect Williamson basic case because the FAB context is invalid.";
        return result;
    }

    DirectLinkTester directLinkTester(prepared, pathTree, metadata);

    const bool aLinkedToF =
        directlyLinkedEitherDirection(
            directLinkTester,
            context.aNode,
            context.fNode
        );

    const bool bLinkedToF =
        directlyLinkedEitherDirection(
            directLinkTester,
            context.bNode,
            context.fNode
        );

    const bool aLinkedToB =
        directlyLinkedEitherDirection(
            directLinkTester,
            context.aNode,
            context.bNode
        );

    if (aLinkedToF && bLinkedToF && aLinkedToB) {
        result.valid = true;
        result.type = WilliamsonBasicCaseType::DirectTriangleLinks;

        // Williamson basic case 1:
        // F dl B dl A dl F
        result.segmentPathNodes.push_back(context.bNode);
        result.segmentPathNodes.push_back(context.aNode);

        result.message =
            "Detected Williamson basic case 1: F dl B dl A dl F.";

        return result;
    }

    result.message =
        "Williamson basic case 1 was not detected. "
        "The next step is SEGFO path construction for the second basic case.";

    return result;
}

bool WilliamsonBasicCaseDetector::directlyLinkedEitherDirection(
    const DirectLinkTester& tester,
    int firstNode,
    int secondNode
) {
    return tester.directlyLinkedToEarlierSegment(firstNode, secondNode)
        || tester.directlyLinkedToEarlierSegment(secondNode, firstNode);
}

} // namespace ht