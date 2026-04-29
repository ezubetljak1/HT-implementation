#pragma once

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
        PathNodeKind kind
    );

    void createNodesFromOrderedDarts();
    void assignParentsAndChildren();
    int findParentNodeForDart(int dartId) const;

    void fillBasicPathData(PathNode& node) const;

    void assignSubtreeIntervals();
    void assignSubtreeIntervalsDfs(int nodeId, int& timer);
};

} // namespace ht