#pragma once

#include <vector>

namespace ht {

struct SegmentMetadata {
    int nodeId = -1;
    int definingDart = -1;
    int parentNode = -1;

    int tailVertex = -1;
    int tailDfsNumber = -1;

    // HEAD(SEG): represented by aggregate information, not by eagerly materialized lists.
    // headCount counts head occurrences from back edges in the segment subtree.
    int headCount = 0;

    int headLow1Dfs = -1;
    int headLow2Dfs = -1;
    int headLow1Vertex = -1;
    int headLow2Vertex = -1;

    // LOW1/LOW2 over RANGE(SEG), where RANGE is represented as:
    // {TAIL(SEG)} union HEAD(SEG), but only low summaries are stored eagerly.
    int low1Dfs = -1;
    int low2Dfs = -1;
    int low1Vertex = -1;
    int low2Vertex = -1;

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