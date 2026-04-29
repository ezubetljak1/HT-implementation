#include "ht/certificate/PathTreeBuilder.hpp"

#include <algorithm>
#include <stdexcept>

namespace ht {

PathTree PathTreeBuilder::build(const PreparedPalmTree& prepared) const {
    PathTreeBuilder builder;
    builder.prepared_ = &prepared;

    builder.initialize();
    builder.buildTreeDartFromParentIndex();
    builder.buildNodesFromOrderedDarts();
    builder.fillSegments();

    return builder.tree_;
}

const Dart& PathTreeBuilder::dart(int dartId) const {
    if (dartId < 0 || dartId >= static_cast<int>(prepared_->darts.size())) {
        throw std::runtime_error("Invalid dart id in PathTreeBuilder.");
    }

    return prepared_->darts[dartId];
}

void PathTreeBuilder::initialize() {
    tree_ = PathTree();

    tree_.nodeByDefiningDart.assign(prepared_->darts.size(), -1);
    tree_.treeDartFromParent.assign(prepared_->n, -1);
}

void PathTreeBuilder::buildTreeDartFromParentIndex() {
    for (const Dart& d : prepared_->darts) {
        if (!d.isTree) {
            continue;
        }

        if (d.to < 0 || d.to >= prepared_->n) {
            throw std::runtime_error("Tree dart has invalid target.");
        }

        if (prepared_->parent[d.to] != d.from) {
            throw std::runtime_error("Tree dart does not match prepared parent relation.");
        }

        if (tree_.treeDartFromParent[d.to] != -1) {
            throw std::runtime_error("Duplicate tree dart for child vertex.");
        }

        tree_.treeDartFromParent[d.to] = d.id;
    }
}

int PathTreeBuilder::createNode(
    int definingDart,
    PathNodeKind kind,
    int parentNode
) {
    PathNode node;
    node.id = static_cast<int>(tree_.nodes.size());
    node.definingDart = definingDart;
    node.kind = kind;
    node.parent = parentNode;

    fillTailHead(node);
    fillCycleData(node);
    fillRangeAndHead(node);
    fillLowValues(node);

    tree_.nodes.push_back(node);

    if (definingDart >= 0) {
        tree_.nodeByDefiningDart[definingDart] = node.id;
    }

    if (parentNode != -1) {
        tree_.nodes[parentNode].children.push_back(node.id);
    }

    return node.id;
}

void PathTreeBuilder::buildNodesFromOrderedDarts() {
    if (prepared_->rootTreeDart == -1) {
        return;
    }

    const int root = createNode(
        prepared_->rootTreeDart,
        PathNodeKind::TreePath,
        -1
    );

    tree_.rootNode = root;

    for (int v = 0; v < prepared_->n; ++v) {
        for (int dartId : prepared_->orderedOut[v]) {
            if (dartId == prepared_->rootTreeDart) {
                continue;
            }

            const Dart& d = dart(dartId);

            PathNodeKind kind =
                d.isTree ? PathNodeKind::TreePath : PathNodeKind::BackPath;

            const int parentNode = findParentNodeForDart(dartId);

            createNode(dartId, kind, parentNode);
        }
    }
}

int PathTreeBuilder::findParentNodeForDart(int dartId) const {
    const Dart& d = dart(dartId);

    if (d.isTree) {
        const int parentVertex = d.from;

        if (parentVertex < 0 || parentVertex >= prepared_->n) {
            return tree_.rootNode;
        }

        const int parentOfParent = prepared_->parent[parentVertex];

        if (parentOfParent == -1) {
            return tree_.rootNode;
        }

        const int parentTreeDart = tree_.treeDartFromParent[parentVertex];

        if (parentTreeDart == -1) {
            return tree_.rootNode;
        }

        const int node = tree_.nodeByDefiningDart[parentTreeDart];

        return node == -1 ? tree_.rootNode : node;
    }

    if (d.isBack) {
        const int sourceVertex = d.from;

        if (sourceVertex < 0 || sourceVertex >= prepared_->n) {
            return tree_.rootNode;
        }

        const int parentTreeDart = tree_.treeDartFromParent[sourceVertex];

        if (parentTreeDart == -1) {
            return tree_.rootNode;
        }

        const int node = tree_.nodeByDefiningDart[parentTreeDart];

        return node == -1 ? tree_.rootNode : node;
    }

    return tree_.rootNode;
}

std::vector<int> PathTreeBuilder::buildTreePathDarts(
    int ancestor,
    int descendant
) const {
    std::vector<int> reversedPath;

    int current = descendant;

    while (current != ancestor) {
        if (current < 0 || current >= prepared_->n) {
            throw std::runtime_error("Invalid vertex while building tree path.");
        }

        const int treeDart = tree_.treeDartFromParent[current];

        if (treeDart == -1) {
            throw std::runtime_error("Missing tree dart while building tree path.");
        }

        reversedPath.push_back(treeDart);

        current = prepared_->parent[current];

        if (current == -1) {
            throw std::runtime_error("Ancestor was not found while building tree path.");
        }
    }

    std::reverse(reversedPath.begin(), reversedPath.end());
    return reversedPath;
}

std::vector<int> PathTreeBuilder::buildCycleDartsForTreeDart(int treeDartId) const {
    const Dart& start = dart(treeDartId);

    if (!start.isTree) {
        throw std::runtime_error("Expected tree dart in buildCycleDartsForTreeDart.");
    }

    std::vector<int> spine;
    spine.push_back(treeDartId);

    int current = start.to;
    int closingBack = -1;
    int w0 = -1;

    while (true) {
        if (current < 0 || current >= prepared_->n) {
            throw std::runtime_error("Invalid vertex while following cycle spine.");
        }

        if (prepared_->orderedOut[current].empty()) {
            throw std::runtime_error("Missing first outgoing dart while building cycle.");
        }

        const int first = prepared_->orderedOut[current].front();
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

    std::vector<int> cycle = buildTreePathDarts(w0, start.from);

    cycle.insert(cycle.end(), spine.begin(), spine.end());
    cycle.push_back(closingBack);

    return cycle;
}

std::vector<int> PathTreeBuilder::buildCycleDartsForBackDart(int backDartId) const {
    const Dart& back = dart(backDartId);

    if (!back.isBack) {
        throw std::runtime_error("Expected back dart in buildCycleDartsForBackDart.");
    }

    std::vector<int> cycle = buildTreePathDarts(back.to, back.from);
    cycle.push_back(backDartId);

    return cycle;
}

void PathTreeBuilder::fillTailHead(PathNode& node) const {
    const Dart& d = dart(node.definingDart);

    node.tailVertex = d.from;
    node.headVertex = d.to;
}

void PathTreeBuilder::fillCycleData(PathNode& node) const {
    const Dart& d = dart(node.definingDart);

    if (d.isTree) {
        node.pathDarts.push_back(node.definingDart);
        node.cycleDarts = buildCycleDartsForTreeDart(node.definingDart);
        return;
    }

    if (d.isBack) {
        node.pathDarts.push_back(node.definingDart);
        node.cycleDarts = buildCycleDartsForBackDart(node.definingDart);
        return;
    }

    throw std::runtime_error("Path node defining dart is neither tree nor back.");
}

void PathTreeBuilder::fillRangeAndHead(PathNode& node) const {
    std::vector<char> seenVertex(static_cast<std::size_t>(prepared_->n), 0);

    for (int dartId : node.cycleDarts) {
        const Dart& d = dart(dartId);

        if (d.from >= 0 && d.from < prepared_->n && !seenVertex[d.from]) {
            seenVertex[d.from] = 1;
            node.rangeVertices.push_back(d.from);
        }

        if (d.to >= 0 && d.to < prepared_->n && !seenVertex[d.to]) {
            seenVertex[d.to] = 1;
            node.rangeVertices.push_back(d.to);
        }
    }

    node.headVertices = node.rangeVertices;

    node.headVertices.erase(
        std::remove(node.headVertices.begin(), node.headVertices.end(), node.tailVertex),
        node.headVertices.end()
    );
}

void PathTreeBuilder::fillLowValues(PathNode& node) const {
    node.low1 = -1;
    node.low2 = -1;
    node.low1Vertex = -1;
    node.low2Vertex = -1;

    for (int vertex : node.headVertices) {
        const int dfs = prepared_->number[vertex];

        if (node.low1 == -1 || dfs < node.low1) {
            node.low2 = node.low1;
            node.low2Vertex = node.low1Vertex;

            node.low1 = dfs;
            node.low1Vertex = vertex;
        } else if (dfs != node.low1 && (node.low2 == -1 || dfs < node.low2)) {
            node.low2 = dfs;
            node.low2Vertex = vertex;
        }
    }
}

void PathTreeBuilder::appendUniqueDart(std::vector<int>& target, int dartId) const {
    for (int existing : target) {
        if (existing == dartId) {
            return;
        }
    }

    target.push_back(dartId);
}

void PathTreeBuilder::collectSegmentDartsDfs(int nodeId, std::vector<int>& output) const {
    const PathNode& node = tree_.nodes[nodeId];

    for (int dartId : node.pathDarts) {
        appendUniqueDart(output, dartId);
    }

    for (int child : node.children) {
        collectSegmentDartsDfs(child, output);
    }
}

void PathTreeBuilder::fillSegments() {
    for (PathNode& node : tree_.nodes) {
        std::vector<int> segmentDarts;
        collectSegmentDartsDfs(node.id, segmentDarts);
        node.segmentDarts = segmentDarts;
    }
}

} // namespace ht