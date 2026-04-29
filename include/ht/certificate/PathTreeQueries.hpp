#pragma once

#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class PathTreeQueries {
public:
    PathTreeQueries(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree
    );

    int nodeForDefiningDart(int dartId) const;

    std::vector<int> treePathDarts(
        int ancestor,
        int descendant
    ) const;

    std::vector<int> cycleDartsForDefiningDart(int dartId) const;
    std::vector<int> cycleDartsForNode(int nodeId) const;

    // Materializes SEG(f) only for one selected node.
    // This uses the subtree interval [preorder, subtreeEnd).
    std::vector<int> segmentDefiningDartsForNode(int nodeId) const;

private:
    const PreparedPalmTree& prepared_;
    const PathTree& pathTree_;

    const Dart& dart(int dartId) const;
    int firstOrderedOut(int vertex) const;

    std::vector<int> cycleDartsForTreeDart(int treeDartId) const;
    std::vector<int> cycleDartsForBackDart(int backDartId) const;
};

} // namespace ht