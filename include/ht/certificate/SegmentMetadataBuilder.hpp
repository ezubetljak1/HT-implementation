#pragma once

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class SegmentMetadataBuilder {
public:
    SegmentMetadataTable build(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree
    ) const;

private:
    const PreparedPalmTree* prepared_ = nullptr;
    const PathTree* pathTree_ = nullptr;

    SegmentMetadataTable table_;

    const Dart& dart(int dartId) const;

    void initializeTable();
    void initializeSegment(const PathNode& node);
    void computeSubtreeHeadSummaries();
    void finalizeRangeLowValues();

    void addHeadCandidate(
        SegmentMetadata& metadata,
        int vertex
    ) const;

    void addRangeCandidate(
        SegmentMetadata& metadata,
        int vertex
    ) const;

    void addDfsCandidate(
        int vertex,
        int dfsNumber,
        int& low1Dfs,
        int& low2Dfs,
        int& low1Vertex,
        int& low2Vertex
    ) const;

    void mergeHeadSummary(
        SegmentMetadata& target,
        const SegmentMetadata& child
    ) const;
};

} // namespace ht