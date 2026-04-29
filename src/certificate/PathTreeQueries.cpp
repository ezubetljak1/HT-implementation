#include "ht/certificate/PathTreeQueries.hpp"

#include <algorithm>
#include <stdexcept>

namespace ht {

PathTreeQueries::PathTreeQueries(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree
) : prepared_(prepared), pathTree_(pathTree) {}

int PathTreeQueries::nodeForDefiningDart(int dartId) const {
    if (dartId < 0 || dartId >= static_cast<int>(pathTree_.nodeByDefiningDart.size())) {
        return -1;
    }

    return pathTree_.nodeByDefiningDart[dartId];
}

const Dart& PathTreeQueries::dart(int dartId) const {
    if (dartId < 0 || dartId >= static_cast<int>(prepared_.darts.size())) {
        throw std::runtime_error("Invalid dart id in PathTreeQueries.");
    }

    return prepared_.darts[dartId];
}

int PathTreeQueries::firstOrderedOut(int vertex) const {
    if (vertex < 0 || vertex >= prepared_.n) {
        throw std::runtime_error("Invalid vertex in firstOrderedOut.");
    }

    if (prepared_.orderedOut[vertex].empty()) {
        throw std::runtime_error("Vertex has no ordered outgoing darts.");
    }

    return prepared_.orderedOut[vertex].front();
}

std::vector<int> PathTreeQueries::treePathDarts(
    int ancestor,
    int descendant
) const {
    std::vector<int> reversedPath;

    int current = descendant;

    while (current != ancestor) {
        if (current < 0 || current >= prepared_.n) {
            throw std::runtime_error("Invalid vertex while materializing tree path.");
        }

        const int treeDart = pathTree_.treeDartFromParent[current];

        if (treeDart == -1) {
            throw std::runtime_error("Missing tree dart while materializing tree path.");
        }

        reversedPath.push_back(treeDart);

        current = prepared_.parent[current];

        if (current == -1) {
            throw std::runtime_error("Ancestor was not found while materializing tree path.");
        }
    }

    std::reverse(reversedPath.begin(), reversedPath.end());

    return reversedPath;
}

std::vector<int> PathTreeQueries::cycleDartsForDefiningDart(int dartId) const {
    const Dart& d = dart(dartId);

    if (d.isTree) {
        return cycleDartsForTreeDart(dartId);
    }

    if (d.isBack) {
        return cycleDartsForBackDart(dartId);
    }

    throw std::runtime_error("Defining dart is neither tree nor back.");
}

std::vector<int> PathTreeQueries::cycleDartsForNode(int nodeId) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(pathTree_.nodes.size())) {
        throw std::runtime_error("Invalid path-tree node id.");
    }

    return cycleDartsForDefiningDart(pathTree_.nodes[nodeId].definingDart);
}

std::vector<int> PathTreeQueries::cycleDartsForTreeDart(int treeDartId) const {
    const Dart& start = dart(treeDartId);

    if (!start.isTree) {
        throw std::runtime_error("Expected tree dart in cycleDartsForTreeDart.");
    }

    std::vector<int> spine;
    spine.push_back(treeDartId);

    int current = start.to;
    int closingBack = -1;
    int w0 = -1;

    while (true) {
        const int first = firstOrderedOut(current);
        const Dart& firstDart = dart(first);

        if (firstDart.isBack) {
            closingBack = first;
            w0 = firstDart.to;
            break;
        }

        if (!firstDart.isTree) {
            throw std::runtime_error("First outgoing dart is neither tree nor back.");
        }

        spine.push_back(first);
        current = firstDart.to;
    }

    std::vector<int> cycle = treePathDarts(w0, start.from);

    cycle.insert(cycle.end(), spine.begin(), spine.end());
    cycle.push_back(closingBack);

    return cycle;
}

std::vector<int> PathTreeQueries::cycleDartsForBackDart(int backDartId) const {
    const Dart& back = dart(backDartId);

    if (!back.isBack) {
        throw std::runtime_error("Expected back dart in cycleDartsForBackDart.");
    }

    std::vector<int> cycle = treePathDarts(back.to, back.from);
    cycle.push_back(backDartId);

    return cycle;
}

std::vector<int> PathTreeQueries::segmentDefiningDartsForNode(int nodeId) const {
    if (nodeId < 0 || nodeId >= static_cast<int>(pathTree_.nodes.size())) {
        throw std::runtime_error("Invalid path-tree node id in segment query.");
    }

    const PathNode& node = pathTree_.nodes[nodeId];

    if (node.preorder < 0 || node.subtreeEnd < node.preorder) {
        throw std::runtime_error("Invalid subtree interval in segment query.");
    }

    if (node.subtreeEnd > static_cast<int>(pathTree_.preorderNodes.size())) {
        throw std::runtime_error("Subtree interval exceeds preorder array.");
    }

    std::vector<int> darts;

    for (int i = node.preorder; i < node.subtreeEnd; ++i) {
        const int currentNodeId = pathTree_.preorderNodes[i];
        const PathNode& currentNode = pathTree_.nodes[currentNodeId];

        for (int dartId : currentNode.pathDarts) {
            darts.push_back(dartId);
        }
    }

    return darts;
}

} // namespace ht