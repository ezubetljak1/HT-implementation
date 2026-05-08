#pragma once

#include <vector>

namespace ht {

struct SegmentHeadWitness {
    int headVertex = -1;      // ancestor / head vertex reached by a back dart
    int headDfs = -1;         // DFS number of headVertex
    int backDart = -1;        // directed back dart: backTailVertex -> headVertex
    int backTailVertex = -1;  // tail of the back dart, inside the segment
};

struct SegmentMetadata {
    int nodeId = -1;
    int definingDart = -1;
    int parentNode = -1;

    int tailVertex = -1;
    int tailDfsNumber = -1;

    // HEAD(SEG): induced by back-edge heads in this segment subtree.
    int headCount = 0;

    int headLow1Dfs = -1;
    int headLow2Dfs = -1;
    int headLow1Vertex = -1;
    int headLow2Vertex = -1;

    SegmentHeadWitness headLow1Witness;
    SegmentHeadWitness headLow2Witness;

    // LOW1/LOW2 over RANGE(SEG) = {TAIL(SEG)} union HEAD(SEG).
    int low1Dfs = -1;
    int low2Dfs = -1;
    int low1Vertex = -1;
    int low2Vertex = -1;

    // Important:
    // If low1/low2 came from HEAD(SEG), these witnesses contain the concrete
    // back dart needed to reconstruct the reduced segment path.
    // If low1/low2 came only from TAIL(SEG), witness.backDart remains -1.
    SegmentHeadWitness low1Witness;
    SegmentHeadWitness low2Witness;

    bool hasHead() const {
        return headCount > 0;
    }

    bool hasLow2() const {
        return low2Dfs != -1;
    }
};

struct SegmentMetadataTable {
    std::vector<SegmentMetadata> segments;

    // path-tree node id -> index in segments.
    // In this implementation segmentByNode[nodeId] == nodeId.
    std::vector<int> segmentByNode;
};

} // namespace ht