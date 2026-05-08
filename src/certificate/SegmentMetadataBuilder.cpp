#include "ht/certificate/SegmentMetadataBuilder.hpp"

#include <stdexcept>

namespace ht {

SegmentMetadataTable SegmentMetadataBuilder::build(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree
) const {
    SegmentMetadataBuilder builder;
    builder.prepared_ = &prepared;
    builder.pathTree_ = &pathTree;

    builder.initializeTable();
    builder.computeSubtreeHeadSummaries();
    builder.finalizeRangeLowValues();

    return builder.table_;
}

const Dart& SegmentMetadataBuilder::dart(int dartId) const {
    if (dartId < 0 || dartId >= static_cast<int>(prepared_->darts.size())) {
        throw std::runtime_error("Invalid dart id in SegmentMetadataBuilder.");
    }

    return prepared_->darts[dartId];
}

void SegmentMetadataBuilder::initializeTable() {
    const int nodeCount = static_cast<int>(pathTree_->nodes.size());

    table_.segments.assign(nodeCount, SegmentMetadata{});
    table_.segmentByNode.assign(nodeCount, -1);

    for (const PathNode& node : pathTree_->nodes) {
        if (node.id < 0 || node.id >= nodeCount) {
            throw std::runtime_error("Invalid path-tree node id in SegmentMetadataBuilder.");
        }

        table_.segmentByNode[node.id] = node.id;
        initializeSegment(node);
    }
}

void SegmentMetadataBuilder::initializeSegment(const PathNode& node) {
    SegmentMetadata metadata;

    metadata.nodeId = node.id;
    metadata.definingDart = node.definingDart;
    metadata.parentNode = node.parent;

    const Dart& defining = dart(node.definingDart);

    metadata.tailVertex = defining.from;

    if (metadata.tailVertex < 0 || metadata.tailVertex >= prepared_->n) {
        throw std::runtime_error("Invalid TAIL vertex in SegmentMetadataBuilder.");
    }

    metadata.tailDfsNumber = prepared_->number[metadata.tailVertex];

    if (metadata.tailDfsNumber <= 0) {
        throw std::runtime_error("Invalid DFS number for TAIL vertex.");
    }

    // In Williamson notation, HEAD(SEG) is induced by back-edge heads
    // in the segment subtree.
    //
    // If this node itself is defined by a back dart f = (tail, head),
    // then the direct head contribution is f.to, and f is the concrete
    // witness edge needed later for REDSEG/direct-link extraction.
    if (defining.isBack) {
        addHeadCandidate(metadata, defining.to, defining.id);
        metadata.headCount = 1;
    }

    table_.segments[node.id] = metadata;
}

void SegmentMetadataBuilder::computeSubtreeHeadSummaries() {
    // PathTreeBuilder already computed preorder intervals.
    // Children appear after parents in preorder, so reverse preorder is enough
    // for aggregating child summaries into parents.
    for (int i = static_cast<int>(pathTree_->preorderNodes.size()) - 1; i >= 0; --i) {
        const int nodeId = pathTree_->preorderNodes[i];

        if (nodeId < 0 || nodeId >= static_cast<int>(pathTree_->nodes.size())) {
            throw std::runtime_error("Invalid node id in path-tree preorder.");
        }

        SegmentMetadata& metadata = table_.segments[nodeId];
        const PathNode& node = pathTree_->nodes[nodeId];

        for (int childNodeId : node.children) {
            if (childNodeId < 0 || childNodeId >= static_cast<int>(table_.segments.size())) {
                throw std::runtime_error("Invalid child node id in SegmentMetadataBuilder.");
            }

            mergeHeadSummary(metadata, table_.segments[childNodeId]);
        }
    }
}

void SegmentMetadataBuilder::finalizeRangeLowValues() {
    for (SegmentMetadata& metadata : table_.segments) {
        metadata.low1Dfs = -1;
        metadata.low2Dfs = -1;
        metadata.low1Vertex = -1;
        metadata.low2Vertex = -1;
        metadata.low1Witness = SegmentHeadWitness{};
        metadata.low2Witness = SegmentHeadWitness{};

        // RANGE(SEG) contains TAIL(SEG).
        // This candidate has no back-edge witness. That is okay:
        // TAIL is a range boundary, not a back-edge head.
        addRangeCandidate(metadata, metadata.tailVertex);

        // RANGE(SEG) also contains HEAD(SEG).
        // These candidates must preserve their concrete back-edge witnesses.
        if (metadata.headLow1Vertex != -1) {
            addRangeCandidateWithWitness(
                metadata,
                metadata.headLow1Vertex,
                metadata.headLow1Witness
            );
        }

        if (metadata.headLow2Vertex != -1) {
            addRangeCandidateWithWitness(
                metadata,
                metadata.headLow2Vertex,
                metadata.headLow2Witness
            );
        }
    }
}

void SegmentMetadataBuilder::addHeadCandidate(
    SegmentMetadata& metadata,
    int vertex,
    int backDart
) const {
    if (vertex < 0 || vertex >= prepared_->n) {
        return;
    }

    const int dfsNumber = prepared_->number[vertex];

    if (dfsNumber <= 0) {
        return;
    }

    SegmentHeadWitness witness;
    witness.headVertex = vertex;
    witness.headDfs = dfsNumber;
    witness.backDart = backDart;

    if (backDart >= 0 && backDart < static_cast<int>(prepared_->darts.size())) {
        witness.backTailVertex = prepared_->darts[backDart].from;
    }

    addDfsCandidateWithWitness(
        vertex,
        dfsNumber,
        witness,
        metadata.headLow1Dfs,
        metadata.headLow2Dfs,
        metadata.headLow1Vertex,
        metadata.headLow2Vertex,
        metadata.headLow1Witness,
        metadata.headLow2Witness
    );
}

void SegmentMetadataBuilder::addRangeCandidate(
    SegmentMetadata& metadata,
    int vertex
) const {
    if (vertex < 0 || vertex >= prepared_->n) {
        return;
    }

    const int dfsNumber = prepared_->number[vertex];

    if (dfsNumber <= 0) {
        return;
    }

    addDfsCandidate(
        vertex,
        dfsNumber,
        metadata.low1Dfs,
        metadata.low2Dfs,
        metadata.low1Vertex,
        metadata.low2Vertex
    );
}

void SegmentMetadataBuilder::addRangeCandidateWithWitness(
    SegmentMetadata& metadata,
    int vertex,
    const SegmentHeadWitness& witness
) const {
    if (vertex < 0 || vertex >= prepared_->n) {
        return;
    }

    const int dfsNumber = prepared_->number[vertex];

    if (dfsNumber <= 0) {
        return;
    }

    addDfsCandidateWithWitness(
        vertex,
        dfsNumber,
        witness,
        metadata.low1Dfs,
        metadata.low2Dfs,
        metadata.low1Vertex,
        metadata.low2Vertex,
        metadata.low1Witness,
        metadata.low2Witness
    );
}

void SegmentMetadataBuilder::addDfsCandidate(
    int vertex,
    int dfsNumber,
    int& low1Dfs,
    int& low2Dfs,
    int& low1Vertex,
    int& low2Vertex
) const {
    if (low1Dfs == dfsNumber || low2Dfs == dfsNumber) {
        return;
    }

    if (low1Dfs == -1 || dfsNumber < low1Dfs) {
        low2Dfs = low1Dfs;
        low2Vertex = low1Vertex;

        low1Dfs = dfsNumber;
        low1Vertex = vertex;
        return;
    }

    if (low2Dfs == -1 || dfsNumber < low2Dfs) {
        low2Dfs = dfsNumber;
        low2Vertex = vertex;
    }
}

void SegmentMetadataBuilder::addDfsCandidateWithWitness(
    int vertex,
    int dfsNumber,
    const SegmentHeadWitness& witness,
    int& low1Dfs,
    int& low2Dfs,
    int& low1Vertex,
    int& low2Vertex,
    SegmentHeadWitness& low1Witness,
    SegmentHeadWitness& low2Witness
) const {
    if (low1Dfs == dfsNumber || low2Dfs == dfsNumber) {
        return;
    }

    if (low1Dfs == -1 || dfsNumber < low1Dfs) {
        low2Dfs = low1Dfs;
        low2Vertex = low1Vertex;
        low2Witness = low1Witness;

        low1Dfs = dfsNumber;
        low1Vertex = vertex;
        low1Witness = witness;
        return;
    }

    if (low2Dfs == -1 || dfsNumber < low2Dfs) {
        low2Dfs = dfsNumber;
        low2Vertex = vertex;
        low2Witness = witness;
    }
}

void SegmentMetadataBuilder::mergeHeadSummary(
    SegmentMetadata& target,
    const SegmentMetadata& child
) const {
    target.headCount += child.headCount;

    if (child.headLow1Vertex != -1) {
        addHeadCandidate(
            target,
            child.headLow1Vertex,
            child.headLow1Witness.backDart
        );
    }

    if (child.headLow2Vertex != -1) {
        addHeadCandidate(
            target,
            child.headLow2Vertex,
            child.headLow2Witness.backDart
        );
    }
}

} // namespace ht