#pragma once

#include <vector>

namespace ht {

struct Segment {
    int id = -1;
    int parentSegmentId = -1;

    // Darts/edges that currently represent this segment.
    // This is intentionally minimal for now.
    std::vector<int> dartIds;

    // DFS numbers of attachments to the parent cycle.
    std::vector<int> attachments;
};

} // namespace ht
