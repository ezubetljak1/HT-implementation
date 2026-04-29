#include "ht/certificate/SegmentForest.hpp"

#include <stdexcept>

namespace ht {

int SegmentForest::addSegment(const Segment& segment) {
    Segment copy = segment;
    copy.id = static_cast<int>(segments_.size());
    segments_.push_back(copy);
    return copy.id;
}

const std::vector<Segment>& SegmentForest::segments() const {
    return segments_;
}

const Segment& SegmentForest::segment(int id) const {
    if (id < 0 || id >= static_cast<int>(segments_.size())) {
        throw std::out_of_range("Segment id is outside segment forest range.");
    }

    return segments_[id];
}

bool SegmentForest::empty() const {
    return segments_.empty();
}

} // namespace ht
