#pragma once

#include <vector>

#include "ht/certificate/Segment.hpp"

namespace ht {

class SegmentForest {
public:
    int addSegment(const Segment& segment);

    const std::vector<Segment>& segments() const;
    const Segment& segment(int id) const;

    bool empty() const;

private:
    std::vector<Segment> segments_;
};

} // namespace ht
