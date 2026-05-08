#include "ht/certificate/WilliamsonFListBuilder.hpp"

namespace ht {

WilliamsonFList WilliamsonFListBuilder::buildFromSegmentList(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonSegmentList& segmentList,
    int fNode
) const {
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

    DirectLinkTester directLinkTester(
        prepared,
        pathTree,
        metadata
    );

    for (int i = 0; i < static_cast<int>(result.segmentNodes.size()); ++i) {
        const int nodeId = result.segmentNodes[i];

        if (nodeId < 0 || nodeId >= static_cast<int>(pathTree.nodes.size())) {
            result.valid = false;
            return result;
        }

        result.positionByNode[nodeId] = i;

        const std::vector<SegmentHeadWitness> witnesses =
            directLinkTester.headWitnessesForNode(nodeId);

        result.headWitnessesByNode[nodeId] = witnesses;

        for (const SegmentHeadWitness& witness : witnesses) {
            const int headVertex = witness.headVertex;

            if (headVertex < 0 || headVertex >= prepared.n) {
                continue;
            }

            result.headsByNode[nodeId].push_back(headVertex);
            result.fxListByVertex[headVertex].push_back(nodeId);
        }
    }

    result.fPosition =
        positionOf(result.positionByNode, fNode);

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

} // namespace ht