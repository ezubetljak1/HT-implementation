#pragma once

#include <vector>

#include "ht/certificate/PathTree.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class PathTreeBuilder {
public:
    PathTree build(const PreparedPalmTree& prepared) const;

private:
    const PreparedPalmTree* prepared_ = nullptr;
    PathTree tree_;

    const Dart& dart(int dartId) const;

    void initialize();
    void buildTreeDartFromParentIndex();

    int createNode(
        int definingDart,
        PathNodeKind kind,
        int parentNode
    );

    void buildNodesFromOrderedDarts();
    int findParentNodeForDart(int dartId) const;

    std::vector<int> buildTreePathDarts(
        int ancestor,
        int descendant
    ) const;

    std::vector<int> buildCycleDartsForTreeDart(int treeDartId) const;
    std::vector<int> buildCycleDartsForBackDart(int backDartId) const;

    void fillTailHead(PathNode& node) const;
    void fillCycleData(PathNode& node) const;
    void fillRangeAndHead(PathNode& node) const;
    void fillLowValues(PathNode& node) const;

    void fillSegments();
    void appendUniqueDart(std::vector<int>& target, int dartId) const;
    void collectSegmentDartsDfs(int nodeId, std::vector<int>& output) const;
};

} // namespace ht