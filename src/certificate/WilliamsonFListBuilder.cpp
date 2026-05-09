#include "ht/certificate/WilliamsonFListBuilder.hpp"

namespace ht {

WilliamsonFList WilliamsonFListBuilder::buildFromSegmentList(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonSegmentList& segmentList,
    int fNode
) const {
    (void) metadata;

    WilliamsonFList result;

    if (!segmentList.valid) {
        return result;
    }

    result.segmentNodes = segmentList.segmentNodes;
    result.positionByNode.assign(pathTree.nodes.size(), -1);
    result.headsByNode.resize(pathTree.nodes.size());
    result.headWitnessesByNode.resize(pathTree.nodes.size());
    result.fxListByVertex.resize(prepared.n);
    result.fNode = fNode;

    std::vector<char> isFListNode(
        pathTree.nodes.size(),
        0
    );

    for (int i = 0; i < static_cast<int>(result.segmentNodes.size()); ++i) {
        const int nodeId = result.segmentNodes[i];

        if (nodeId < 0 || nodeId >= static_cast<int>(pathTree.nodes.size())) {
            result.valid = false;
            return result;
        }

        result.positionByNode[nodeId] = i;
        isFListNode[nodeId] = 1;
    }

    // Linear preorder sweep over the PathTree.
    //
    // This replaces the old pattern:
    //
    //     for each segment:
    //         recursively scan its subtree
    //
    // That old approach can become quadratic.
    //
    // Here, each path-tree node is visited once. While visiting a node,
    // all currently active FLIST segment roots are exactly the selected
    // segment subtrees that contain this node.
    std::vector<ActiveSegment> activeSegments;

    for (int preorderIndex = 0;
         preorderIndex < static_cast<int>(pathTree.preorderNodes.size());
         ++preorderIndex) {
        const int nodeId = pathTree.preorderNodes[preorderIndex];

        if (nodeId < 0 || nodeId >= static_cast<int>(pathTree.nodes.size())) {
            result.valid = false;
            return result;
        }

        while (!activeSegments.empty() &&
               preorderIndex >= activeSegments.back().subtreeEnd) {
            activeSegments.pop_back();
        }

        if (isFListNode[nodeId]) {
            ActiveSegment active;
            active.nodeId = nodeId;
            active.subtreeEnd = pathTree.nodes[nodeId].subtreeEnd;
            activeSegments.push_back(active);
        }

        std::vector<SegmentHeadWitness> nodeWitnesses;

        addWitnessesFromPathNode(
            prepared,
            pathTree,
            nodeId,
            nodeWitnesses
        );

        if (nodeWitnesses.empty()) {
            continue;
        }

        for (const ActiveSegment& active : activeSegments) {
            for (const SegmentHeadWitness& witness : nodeWitnesses) {
                addWitnessToSegment(
                    prepared,
                    result,
                    active.nodeId,
                    witness
                );
            }
        }
    }

    result.fPosition =
        positionOf(
            result.positionByNode,
            fNode
        );

    result.valid = result.fPosition != -1;

    return result;
}

int WilliamsonFListBuilder::positionOf(
    const std::vector<int>& positionByNode,
    int nodeId
) {
    if (nodeId < 0 || nodeId >= static_cast<int>(positionByNode.size())) {
        return -1;
    }

    return positionByNode[nodeId];
}

void WilliamsonFListBuilder::addWitnessToSegment(
    const PreparedPalmTree& prepared,
    WilliamsonFList& result,
    int segmentNode,
    const SegmentHeadWitness& witness
) {
    if (segmentNode < 0 ||
        segmentNode >= static_cast<int>(result.headWitnessesByNode.size())) {
        return;
    }

    if (witness.headVertex < 0 ||
        witness.headVertex >= prepared.n ||
        witness.backDart < 0 ||
        witness.backTailVertex < 0) {
        return;
    }

    // Preserve old DirectLinkTester behavior:
    // one HEAD vertex per segment.
    for (const SegmentHeadWitness& existing :
         result.headWitnessesByNode[segmentNode]) {
        if (existing.headVertex == witness.headVertex) {
            return;
        }
    }

    result.headWitnessesByNode[segmentNode].push_back(witness);
    result.headsByNode[segmentNode].push_back(witness.headVertex);

    result.fxListByVertex[witness.headVertex].push_back(segmentNode);
}

void WilliamsonFListBuilder::addWitnessesFromPathNode(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    int pathNodeId,
    std::vector<SegmentHeadWitness>& witnesses
) {
    if (pathNodeId < 0 ||
        pathNodeId >= static_cast<int>(pathTree.nodes.size())) {
        return;
    }

    const PathNode& node = pathTree.nodes[pathNodeId];

    // Important:
    // Use node.pathDarts, not only node.definingDart.
    // This matches the old DirectLinkTester traversal semantics.
    for (int dartId : node.pathDarts) {
        const SegmentHeadWitness witness =
            witnessFromBackDart(
                prepared,
                dartId
            );

        if (witness.backDart != -1) {
            witnesses.push_back(witness);
        }
    }
}

SegmentHeadWitness WilliamsonFListBuilder::witnessFromBackDart(
    const PreparedPalmTree& prepared,
    int dartId
) {
    SegmentHeadWitness witness;

    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        return witness;
    }

    const Dart& dart = prepared.darts[dartId];

    if (!dart.isBack) {
        return witness;
    }

    if (dart.from < 0 || dart.from >= prepared.n ||
        dart.to < 0 || dart.to >= prepared.n) {
        return witness;
    }

    witness.headVertex = dart.to;
    witness.headDfs = prepared.number[dart.to];
    witness.backDart = dartId;
    witness.backTailVertex = dart.from;

    return witness;
}

} // namespace ht