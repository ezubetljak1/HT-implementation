#include "ht/certificate/WilliamsonContextBuilder.hpp"

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

    int cycleNode =
        nodeForDart(
            pathTree,
            failure.cycleRootDart
        );

    if (cycleNode == -1) {
        cycleNode =
            nodeForDart(
                pathTree,
                failure.closingBackDart
            );
    }

    if (cycleNode == -1) {
        std::ostringstream out;
        out << "Could not map failure cycle to a PathTree node. "
            << "cycleRootDart=" << failure.cycleRootDart
            << ", closingBackDart=" << failure.closingBackDart;
        context.message = out.str();
        return context;
    }

    if (failure.type == StrongPlanarityFailureType::BothSidesAttachAboveW0) {
        context =
            buildBothSidesAttachAboveW0Context(
                prepared,
                pathTree,
                metadata,
                failure,
                cycleNode
            );

        if (context.valid) {
            return context;
        }

        // Do not stop here. If the specialized mapping fails, fall back to the
        // generic mapping, but keep the specialized error in the final message.
        WilliamsonContext generic =
            buildGenericContext(
                prepared,
                pathTree,
                metadata,
                failure,
                cycleNode
            );

        if (generic.valid) {
            generic.message =
                "Built Williamson context using generic fallback after "
                "BothSidesAttachAboveW0-specific mapping failed. Specific failure: " +
                context.message;
            return generic;
        }

        generic.message =
            "BothSidesAttachAboveW0-specific mapping failed: " +
            context.message +
            " Generic mapping also failed: " +
            generic.message;

        return generic;
    }

    return buildGenericContext(
        prepared,
        pathTree,
        metadata,
        failure,
        cycleNode
    );
}

WilliamsonContext WilliamsonContextBuilder::buildBothSidesAttachAboveW0Context(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const StrongPlanarityFailure& failure,
    int cycleNode
) {
    (void) prepared;

    WilliamsonContext context;

    DirectLinkTester tester(
        prepared,
        pathTree,
        metadata
    );

    std::vector<int> fNodes;

    // For BothSidesAttachAboveW0, the most meaningful F candidates are the
    // segments emanating from the cycle root. This is the segment whose
    // placement forces the both-sides-above-w0 conflict.
    for (int dartId : failure.cycleRootEmanatingDarts) {
        addUnique(
            fNodes,
            nodeForDart(
                pathTree,
                dartId
            )
        );
    }

    // Conservative fallback candidates. These are still constant-size.
    addUnique(
        fNodes,
        nodeForDart(
            pathTree,
            failure.cycleRootDart
        )
    );

    addUnique(
        fNodes,
        nodeForDart(
            pathTree,
            failure.closingBackDart
        )
    );

    addUnique(
        fNodes,
        nodeForDart(
            pathTree,
            failure.rootTreeDart
        )
    );

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
        out << "BothSidesAttachAboveW0 mapping could not collect both side lists. "
            << "leftCandidates=" << leftCandidates.size()
            << ", rightCandidates=" << rightCandidates.size()
            << ", blockLeftSegments=" << failure.blockLeftSegments.size()
            << ", blockRightSegments=" << failure.blockRightSegments.size()
            << ", stackTopLeftSegments=" << failure.stackTopLeftSegments.size()
            << ", stackTopRightSegments=" << failure.stackTopRightSegments.size();
        context.message = out.str();
        return context;
    }

    for (int fNode : fNodes) {
        if (fNode == -1 || fNode == cycleNode) {
            continue;
        }

        // Direction matters:
        //
        //     A dl F means HEAD(A) intersects OSPAN(F).
        //     B dl F means HEAD(B) intersects OSPAN(F).
        //
        // Therefore source = A/B, span = F.
        const int aNode =
            findFirstNodeLinkingToF(
                tester,
                fNode,
                leftCandidates
            );

        const int bNode =
            findFirstNodeLinkingToF(
                tester,
                fNode,
                rightCandidates
            );

        if (aNode == -1 || bNode == -1) {
            continue;
        }

        WilliamsonContext mapped =
            makeContext(
                pathTree,
                tester,
                fNode,
                aNode,
                bNode,
                cycleNode
            );

        if (mapped.valid) {
            mapped.message =
                "Mapped BothSidesAttachAboveW0 failure to Williamson context "
                "using directed links A dl F and B dl F.";
            return mapped;
        }
    }

    std::ostringstream out;
    out << "BothSidesAttachAboveW0 mapping failed to find directed A/B links to F. "
        << "fCandidates=" << fNodes.size()
        << ", leftCandidates=" << leftCandidates.size()
        << ", rightCandidates=" << rightCandidates.size()
        << ", cycleNode=" << cycleNode
        << ", cycleRootDart=" << failure.cycleRootDart
        << ", closingBackDart=" << failure.closingBackDart
        << ", cycleRootEmanatingDarts=" << failure.cycleRootEmanatingDarts.size();

    context.message = out.str();
    return context;
}

WilliamsonContext WilliamsonContextBuilder::buildGenericContext(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const StrongPlanarityFailure& failure,
    int cycleNode
) {
    WilliamsonContext context;

    DirectLinkTester tester(
        prepared,
        pathTree,
        metadata
    );

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

    std::vector<int> fDartCandidates =
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

    // Still allow side-list nodes as F candidates, but we no longer validate
    // through the old "appears around cycle" heuristic.
    for (int nodeId : leftCandidates) {
        addUnique(fNodeCandidates, nodeId);
    }

    for (int nodeId : rightCandidates) {
        addUnique(fNodeCandidates, nodeId);
    }

    for (int fNode : fNodeCandidates) {
        if (fNode == -1 || fNode == cycleNode) {
            continue;
        }

        const int aNode =
            findFirstNodeLinkingToF(
                tester,
                fNode,
                leftCandidates
            );

        const int bNode =
            findFirstNodeLinkingToF(
                tester,
                fNode,
                rightCandidates
            );

        if (aNode == -1 || bNode == -1) {
            continue;
        }

        WilliamsonContext mapped =
            makeContext(
                pathTree,
                tester,
                fNode,
                aNode,
                bNode,
                cycleNode
            );

        if (mapped.valid) {
            mapped.message =
                "Mapped strong-planarity failure to Williamson context "
                "using directed A/B links to F.";
            return mapped;
        }
    }

    std::ostringstream out;
    out << "Could not map failure to Williamson context (F, A, B). "
        << "fNodeCandidates=" << fNodeCandidates.size()
        << ", leftCandidates=" << leftCandidates.size()
        << ", rightCandidates=" << rightCandidates.size()
        << ", cycleNode=" << cycleNode;

    context.message = out.str();
    return context;
}

int WilliamsonContextBuilder::nodeForDart(
    const PathTree& pathTree,
    int dartId
) {
    if (dartId < 0 ||
        dartId >= static_cast<int>(pathTree.nodeByDefiningDart.size())) {
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

    for (int dartId : failure.cycleRootEmanatingDarts) {
        addUnique(candidates, dartId);
    }

    addUnique(candidates, failure.cycleRootDart);
    addUnique(candidates, failure.closingBackDart);
    addUnique(candidates, failure.currentDart);
    addUnique(candidates, failure.rootTreeDart);

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
        addUnique(
            nodes,
            nodeForDart(
                pathTree,
                dartId
            )
        );
    }
}

bool WilliamsonContextBuilder::segmentLinksToSpan(
    const DirectLinkTester& tester,
    int sourceNode,
    int spanNode
) {
    return tester.findPathFromSegmentToOpenSpan(
        sourceNode,
        spanNode
    ).exists;
}

int WilliamsonContextBuilder::findFirstNodeLinkingToF(
    const DirectLinkTester& tester,
    int fNode,
    const std::vector<int>& candidateNodes
) {
    for (int candidateNode : candidateNodes) {
        if (candidateNode == -1 || candidateNode == fNode) {
            continue;
        }

        if (segmentLinksToSpan(
                tester,
                candidateNode,
                fNode
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

    if (fNode < 0 || fNode >= static_cast<int>(pathTree.nodes.size()) ||
        aNode < 0 || aNode >= static_cast<int>(pathTree.nodes.size()) ||
        bNode < 0 || bNode >= static_cast<int>(pathTree.nodes.size()) ||
        cycleNode < 0 || cycleNode >= static_cast<int>(pathTree.nodes.size())) {
        context.message = "Invalid node id while creating Williamson context.";
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

    // Direction matters.
    //
    // A dl F and B dl F mean:
    //
    //     HEAD(A) intersects OSPAN(F)
    //     HEAD(B) intersects OSPAN(F)
    //
    // so A/B are source nodes and F is the span node.
    context.aLinkedToF =
        segmentLinksToSpan(
            tester,
            context.aNode,
            context.fNode
        );

    context.bLinkedToF =
        segmentLinksToSpan(
            tester,
            context.bNode,
            context.fNode
        );

    context.valid =
        context.aLinkedToF &&
        context.bLinkedToF &&
        context.fNode != -1 &&
        context.aNode != -1 &&
        context.bNode != -1 &&
        context.cycleNode != -1;

    if (context.valid) {
        context.message =
            "Mapped strong-planarity failure to Williamson context "
            "with directed links A dl F and B dl F.";
    } else {
        context.message =
            "Williamson context mapping failed directed-link validation.";
    }

    return context;
}

} // namespace ht