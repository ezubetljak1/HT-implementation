#include "ht/certificate/PathTreeBuilder.hpp"

#include <stdexcept>

namespace ht {

PathTree PathTreeBuilder::build(const PreparedPalmTree& prepared) const {
    PathTreeBuilder builder;
    builder.prepared_ = &prepared;

    builder.initialize();
    builder.buildTreeDartFromParentIndex();
    builder.createNodesFromOrderedDarts();
    builder.assignParentsAndChildren();
    builder.assignSubtreeIntervals();

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
    PathNodeKind kind
) {
    if (definingDart < 0 || definingDart >= static_cast<int>(prepared_->darts.size())) {
        throw std::runtime_error("Invalid defining dart in createNode.");
    }

    if (tree_.nodeByDefiningDart[definingDart] != -1) {
        return tree_.nodeByDefiningDart[definingDart];
    }

    PathNode node;
    node.id = static_cast<int>(tree_.nodes.size());
    node.definingDart = definingDart;
    node.kind = kind;

    fillBasicPathData(node);

    tree_.nodes.push_back(node);
    tree_.nodeByDefiningDart[definingDart] = node.id;

    return node.id;
}

void PathTreeBuilder::createNodesFromOrderedDarts() {
    if (prepared_->rootTreeDart == -1) {
        return;
    }

    tree_.rootNode = createNode(
        prepared_->rootTreeDart,
        PathNodeKind::TreePath
    );

    for (int v = 0; v < prepared_->n; ++v) {
        for (int dartId : prepared_->orderedOut[v]) {
            if (dartId == prepared_->rootTreeDart) {
                continue;
            }

            const Dart& d = dart(dartId);

            if (!d.isTree && !d.isBack) {
                continue;
            }

            PathNodeKind kind =
                d.isTree ? PathNodeKind::TreePath : PathNodeKind::BackPath;

            createNode(dartId, kind);
        }
    }
}

void PathTreeBuilder::assignParentsAndChildren() {
    if (tree_.rootNode == -1) {
        return;
    }

    for (PathNode& node : tree_.nodes) {
        if (node.id == tree_.rootNode) {
            node.parent = -1;
            continue;
        }

        node.parent = findParentNodeForDart(node.definingDart);

        if (node.parent < 0 || node.parent >= static_cast<int>(tree_.nodes.size())) {
            node.parent = tree_.rootNode;
        }

        if (node.parent == node.id) {
            node.parent = tree_.rootNode;
        }
    }

    for (PathNode& node : tree_.nodes) {
        node.children.clear();
    }

    for (const PathNode& node : tree_.nodes) {
        if (node.parent != -1) {
            tree_.nodes[node.parent].children.push_back(node.id);
        }
    }
}

int PathTreeBuilder::findParentNodeForDart(int dartId) const {
    const Dart& d = dart(dartId);

    if (tree_.rootNode == -1) {
        return -1;
    }

    if (d.isTree) {
        const int parentVertex = d.from;

        if (parentVertex < 0 || parentVertex >= prepared_->n) {
            return tree_.rootNode;
        }

        if (prepared_->parent[parentVertex] == -1) {
            return tree_.rootNode;
        }

        const int parentTreeDart = tree_.treeDartFromParent[parentVertex];

        if (parentTreeDart == -1) {
            return tree_.rootNode;
        }

        const int parentNode = tree_.nodeByDefiningDart[parentTreeDart];

        return parentNode == -1 ? tree_.rootNode : parentNode;
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

        const int parentNode = tree_.nodeByDefiningDart[parentTreeDart];

        return parentNode == -1 ? tree_.rootNode : parentNode;
    }

    return tree_.rootNode;
}

void PathTreeBuilder::fillBasicPathData(PathNode& node) const {
    const Dart& d = dart(node.definingDart);

    node.tailVertex = d.from;
    node.headVertex = d.to;

    node.pathDarts.clear();
    node.pathDarts.push_back(node.definingDart);

    // Keep these empty in the linear builder.
    // They should be materialized only for a selected failure/certificate case.
    node.cycleDarts.clear();
    node.segmentDarts.clear();
    node.rangeVertices.clear();
    node.headVertices.clear();

    node.low1 = -1;
    node.low2 = -1;
    node.low1Vertex = -1;
    node.low2Vertex = -1;
}

void PathTreeBuilder::assignSubtreeIntervals() {
    tree_.preorderNodes.clear();

    if (tree_.rootNode == -1) {
        return;
    }

    int timer = 0;
    assignSubtreeIntervalsDfs(tree_.rootNode, timer);
}

void PathTreeBuilder::assignSubtreeIntervalsDfs(int nodeId, int& timer) {
    if (nodeId < 0 || nodeId >= static_cast<int>(tree_.nodes.size())) {
        throw std::runtime_error("Invalid path-tree node id during DFS interval assignment.");
    }

    PathNode& node = tree_.nodes[nodeId];

    node.preorder = timer++;
    tree_.preorderNodes.push_back(nodeId);

    for (int child : node.children) {
        assignSubtreeIntervalsDfs(child, timer);
    }

    node.subtreeEnd = timer;
}

} // namespace ht