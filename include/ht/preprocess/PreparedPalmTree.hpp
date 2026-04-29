#pragma once

#include <vector>

#include "ht/Types.hpp"

namespace ht {

struct PreparedPalmTree {
    int n = 0;
    int edgeCount = 0;

    // First tree dart that exits the DFS root.
    int rootTreeDart = -1;

    std::vector<int> localToOriginal;
    std::vector<int> number;
    std::vector<int> parent;

    // All darts, exactly two per undirected edge.
    std::vector<Dart> darts;

    // All outgoing darts from vertex v, including reverse/non-active darts.
    std::vector<std::vector<int>> outAll;

    // Only active HT orientation, sorted by phi:
    // - tree darts parent -> child
    // - back/frond darts descendant -> ancestor
    std::vector<std::vector<int>> orderedOut;

    // Side assignment produced by strong-planarity.
    std::vector<Side> alpha;
};

} // namespace ht
