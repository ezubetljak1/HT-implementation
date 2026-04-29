#pragma once

#include <vector>

#include "ht/certificate/DirectLinkTester.hpp"
#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

namespace ht {

class WilliamsonContextBuilder {
public:
    WilliamsonContext build(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const StrongPlanarityFailure& failure
    ) const;

private:
    static int nodeForDart(
        const PathTree& pathTree,
        int dartId
    );

    static void addUnique(
        std::vector<int>& values,
        int value
    );

    static std::vector<int> collectFDartCandidates(
        const StrongPlanarityFailure& failure
    );

    static std::vector<int> collectCandidateNodes(
        const PathTree& pathTree,
        const std::vector<int>& dartIds
    );

    static void appendCandidateNodes(
        std::vector<int>& nodes,
        const PathTree& pathTree,
        const std::vector<int>& dartIds
    );

    static bool directlyLinkedEitherDirection(
        const DirectLinkTester& tester,
        int firstNode,
        int secondNode
    );

    static int findFirstNodeLinkedToF(
        const DirectLinkTester& tester,
        int fNode,
        const std::vector<int>& candidateNodes
    );

    static WilliamsonContext makeContext(
        const PathTree& pathTree,
        const DirectLinkTester& tester,
        int fNode,
        int aNode,
        int bNode,
        int cycleNode
    );

    static bool containsNode(
        const std::vector<int>& nodes,
        int nodeId
    );

    static std::vector<int> collectSegmentNodesAroundCycle(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        int cycleNode
    );

    static bool contextNodesAppearAroundCycle(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        int cycleNode,
        int fNode,
        int aNode,
        int bNode
    );
};

} // namespace ht