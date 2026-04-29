#pragma once

#include <vector>

namespace ht {

enum class PathNodeKind {
    TreePath,
    BackPath
};

struct PathNode {
    int id = -1;

    // The dart f that defines PATH(f).
    int definingDart = -1;

    PathNodeKind kind = PathNodeKind::TreePath;

    // Parent in PATR(G,T). -1 for root path.
    int parent = -1;

    // Children in PATR(G,T), this is SEGLIST(definingDart) at path-node level.
    std::vector<int> children;

    // PATH(f). For now this is represented by the defining dart.
    // Total size over all nodes is O(E).
    std::vector<int> pathDarts;

    // IMPORTANT:
    // These are intentionally not eagerly materialized by PathTreeBuilder.
    // Eagerly building CYCLE(f) and SEG(f) for every node can become non-linear.
    // They are kept as optional/debug fields for later targeted certificate extraction.
    std::vector<int> cycleDarts;
    std::vector<int> segmentDarts;

    // TAIL(f) and HEAD(f) in Williamson-style notation.
    int tailVertex = -1;
    int headVertex = -1;

    // RANGE/HEAD sets are also not eagerly materialized for every node.
    std::vector<int> rangeVertices;
    std::vector<int> headVertices;

    int low1 = -1;
    int low2 = -1;
    int low1Vertex = -1;
    int low2Vertex = -1;

    // Linear implicit representation of SEG(f):
    // SEG(f) corresponds to the subtree interval
    // preorderNodes[preorder, subtreeEnd).
    int preorder = -1;
    int subtreeEnd = -1;
};

struct PathTree {
    std::vector<PathNode> nodes;

    // dart id -> path node id, if this dart defines a PATH(f)
    std::vector<int> nodeByDefiningDart;

    // vertex -> tree dart from parent to this vertex
    std::vector<int> treeDartFromParent;

    // DFS preorder of the path tree nodes.
    std::vector<int> preorderNodes;

    int rootNode = -1;
};

} // namespace ht