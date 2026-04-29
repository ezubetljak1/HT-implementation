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

    // Parent in PATR(G,T). -1 for artificial/root path.
    int parent = -1;

    // Children in PATR(G,T), this is SEGLIST(definingDart) at the path-node level.
    std::vector<int> children;

    // PATH(f): directed darts forming the path.
    std::vector<int> pathDarts;

    // CYCLE(f): tree path + closing back edge.
    std::vector<int> cycleDarts;

    // SEG(f): PATH(f) plus all descendants. Filled after tree is built.
    std::vector<int> segmentDarts;

    // TAIL(f) and HEAD(f) in Williamson's notation.
    int tailVertex = -1;
    int headVertex = -1;

    // RANGE/HEAD vertices on parent cycle. Initially approximate but explicit.
    std::vector<int> rangeVertices;
    std::vector<int> headVertices;

    int low1 = -1;
    int low2 = -1;
    int low1Vertex = -1;
    int low2Vertex = -1;
};

struct PathTree {
    std::vector<PathNode> nodes;

    // dart id -> path node id, if this dart defines a PATH(f)
    std::vector<int> nodeByDefiningDart;

    // vertex -> tree dart from parent to this vertex
    std::vector<int> treeDartFromParent;

    int rootNode = -1;
};

} // namespace ht