#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"

#include "ht/certificate/PathTreeQueries.hpp"

#include <sstream>

namespace ht {

WilliamsonSegmentList WilliamsonSegmentListBuilder::build(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const WilliamsonContext& context
) const {
    WilliamsonSegmentList result;

    if (!context.valid) {
        result.message =
            "Cannot build SEGLIST(e) from invalid Williamson context.";
        return result;
    }

    if (context.cycleNode < 0
        || context.cycleNode >= static_cast<int>(pathTree.nodes.size())) {
        result.message =
            "Invalid context.cycleNode while building SEGLIST(e).";
        return result;
    }

    result.baseNode = context.cycleNode;
    result.positionByNode.assign(pathTree.nodes.size(), -1);

    PathTreeQueries queries(prepared, pathTree);

    std::vector<int> cycleDarts =
        queries.cycleDartsForNode(context.cycleNode);

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
            result.message = "CYCLE(e) contains invalid dart id.";
            return result;
        }

        dartIsOnBaseCycle[dartId] = 1;

        const int reverseDart = prepared.darts[dartId].rev;

        if (reverseDart >= 0
            && reverseDart < static_cast<int>(dartIsOnBaseCycle.size())) {
            dartIsOnBaseCycle[reverseDart] = 1;
        }

        const Dart& dart = prepared.darts[dartId];

        addUniqueCycleVertex(
            prepared,
            dart.from,
            seenCycleVertex,
            cycleVertices
        );

        addUniqueCycleVertex(
            prepared,
            dart.to,
            seenCycleVertex,
            cycleVertices
        );
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
            addNodeForDartAndReverse(
                prepared,
                pathTree,
                dartId,
                dartIsOnBaseCycle,
                seenNode,
                result.segmentNodes
            );
        }
    }

    for (int i = 0;
         i < static_cast<int>(result.segmentNodes.size());
         ++i) {
        const int nodeId = result.segmentNodes[i];

        if (nodeId < 0 || nodeId >= static_cast<int>(result.positionByNode.size())) {
            result.message =
                "SEGLIST(e) contains invalid PathTree node id.";
            return result;
        }

        result.positionByNode[nodeId] = i;
    }

    result.fPosition =
        positionOf(
            result.positionByNode,
            context.fNode
        );

    result.aPosition =
        positionOf(
            result.positionByNode,
            context.aNode
        );

    result.bPosition =
        positionOf(
            result.positionByNode,
            context.bNode
        );

    result.valid =
        result.fPosition != -1
        && result.aPosition != -1
        && result.bPosition != -1;

    if (result.valid) {
        result.message =
            "Built Williamson SEGLIST(e) containing F, A and B.";
    } else {
        std::ostringstream out;

        out << "F, A and B are not all present in the computed SEGLIST(e). "
            << "segmentNodes=" << result.segmentNodes.size()
            << ", fNode=" << context.fNode
            << ", aNode=" << context.aNode
            << ", bNode=" << context.bNode
            << ", fPosition=" << result.fPosition
            << ", aPosition=" << result.aPosition
            << ", bPosition=" << result.bPosition;

        result.message = out.str();
    }

    return result;
}

int WilliamsonSegmentListBuilder::positionOf(
    const std::vector<int>& positionByNode,
    int nodeId
) {
    if (nodeId < 0 || nodeId >= static_cast<int>(positionByNode.size())) {
        return -1;
    }

    return positionByNode[nodeId];
}

void WilliamsonSegmentListBuilder::addUniqueCycleVertex(
    const PreparedPalmTree& prepared,
    int vertex,
    std::vector<char>& seenVertex,
    std::vector<int>& cycleVertices
) {
    if (vertex < 0 || vertex >= prepared.n) {
        return;
    }

    if (seenVertex[vertex]) {
        return;
    }

    seenVertex[vertex] = 1;
    cycleVertices.push_back(vertex);
}

void WilliamsonSegmentListBuilder::addUniqueSegmentNode(
    const PathTree& pathTree,
    int nodeId,
    std::vector<char>& seenNode,
    std::vector<int>& segmentNodes
) {
    if (nodeId < 0 || nodeId >= static_cast<int>(pathTree.nodes.size())) {
        return;
    }

    if (seenNode[nodeId]) {
        return;
    }

    seenNode[nodeId] = 1;
    segmentNodes.push_back(nodeId);
}

void WilliamsonSegmentListBuilder::addNodeForDartAndReverse(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    int dartId,
    const std::vector<char>& dartIsOnBaseCycle,
    std::vector<char>& seenNode,
    std::vector<int>& segmentNodes
) {
    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        return;
    }

    if (dartIsOnBaseCycle[dartId]) {
        return;
    }

    const int directNode =
        nodeForDart(
            pathTree,
            dartId
        );

    addUniqueSegmentNode(
        pathTree,
        directNode,
        seenNode,
        segmentNodes
    );

    const int reverseDart =
        prepared.darts[dartId].rev;

    if (reverseDart < 0
        || reverseDart >= static_cast<int>(prepared.darts.size())) {
        return;
    }

    if (dartIsOnBaseCycle[reverseDart]) {
        return;
    }

    const int reverseNode =
        nodeForDart(
            pathTree,
            reverseDart
        );

    addUniqueSegmentNode(
        pathTree,
        reverseNode,
        seenNode,
        segmentNodes
    );
}

int WilliamsonSegmentListBuilder::nodeForDart(
    const PathTree& pathTree,
    int dartId
) {
    if (dartId < 0
        || dartId >= static_cast<int>(pathTree.nodeByDefiningDart.size())) {
        return -1;
    }

    return pathTree.nodeByDefiningDart[dartId];
}

} // namespace ht