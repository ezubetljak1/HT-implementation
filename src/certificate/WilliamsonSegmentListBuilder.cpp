#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"

#include "ht/certificate/PathTreeQueries.hpp"

namespace ht {

WilliamsonSegmentList WilliamsonSegmentListBuilder::build(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const WilliamsonContext& context
) const {
    WilliamsonSegmentList result;

    if (!context.valid) {
        result.message = "Cannot build SEGLIST(e) from invalid Williamson context.";
        return result;
    }

    if (context.cycleNode < 0 || context.cycleNode >= static_cast<int>(pathTree.nodes.size())) {
        result.message = "Invalid context.cycleNode while building SEGLIST(e).";
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

    // SEGLIST(e): segments attached to vertices of CYCLE(e), in the order
    // induced by the cycle vertices and prepared ordered outgoing darts.
    // We skip darts that are part of the base cycle itself.
    for (int vertex : cycleVertices) {
        if (vertex < 0 || vertex >= prepared.n) {
            continue;
        }

        for (int dartId : prepared.orderedOut[vertex]) {
            if (dartId < 0 || dartId >= static_cast<int>(pathTree.nodeByDefiningDart.size())) {
                continue;
            }

            if (dartIsOnBaseCycle[dartId]) {
                continue;
            }

            const int nodeId = pathTree.nodeByDefiningDart[dartId];

            if (nodeId == -1) {
                continue;
            }

            if (nodeId < 0 || nodeId >= static_cast<int>(pathTree.nodes.size())) {
                result.message = "SEGLIST(e) candidate has invalid PathTree node id.";
                return result;
            }

            if (seenNode[nodeId]) {
                continue;
            }

            seenNode[nodeId] = 1;
            result.segmentNodes.push_back(nodeId);
        }
    }

    for (int i = 0; i < static_cast<int>(result.segmentNodes.size()); ++i) {
        const int nodeId = result.segmentNodes[i];
        result.positionByNode[nodeId] = i;
    }

    result.fPosition = positionOf(result.positionByNode, context.fNode);
    result.aPosition = positionOf(result.positionByNode, context.aNode);
    result.bPosition = positionOf(result.positionByNode, context.bNode);

    result.valid =
        result.fPosition != -1
        && result.aPosition != -1
        && result.bPosition != -1;

    if (result.valid) {
        result.message = "Built Williamson SEGLIST(e) containing F, A and B.";
    } else {
        result.message =
            "F, A and B are not all present in the computed SEGLIST(e).";
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

} // namespace ht