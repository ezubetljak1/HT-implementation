#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"

using namespace ht;

namespace {

PreparedPalmTree prepareSingleComponent(const Graph& graph) {
    BiconnectedComponentsFinder finder;
    Components components = finder.find(graph);

    assert(components.size() == 1);

    ComponentPreprocessor preprocessor;
    PreprocessedComponent pc = preprocessor.preprocess(components[0]);

    PreparedPalmTreeBuilder builder;
    return builder.build(pc);
}

} // namespace

HT_TEST(PathTreeBuilderBuildsNodesForK33) {
    Graph g = ht::test::buildK33();

    PreparedPalmTree prepared = prepareSingleComponent(g);

    PathTreeBuilder builder;
    PathTree tree = builder.build(prepared);

    assert(tree.rootNode != -1);
    assert(!tree.nodes.empty());
    assert(tree.nodeByDefiningDart.size() == prepared.darts.size());
    assert(tree.preorderNodes.size() == tree.nodes.size());

    for (const PathNode& node : tree.nodes) {
        assert(node.id >= 0);
        assert(node.definingDart >= 0);
        assert(!node.pathDarts.empty());
        assert(node.tailVertex >= 0);
        assert(node.headVertex >= 0);

        assert(node.preorder >= 0);
        assert(node.subtreeEnd > node.preorder);
        assert(node.subtreeEnd <= static_cast<int>(tree.nodes.size()));

        // These are intentionally lazy/not materialized by the linear builder.
        assert(node.cycleDarts.empty());
        assert(node.segmentDarts.empty());
    }
}

HT_TEST(PathTreeBuilderBuildsNodesForSubdividedK5) {
    Graph g = ht::test::buildSubdividedK5();

    PreparedPalmTree prepared = prepareSingleComponent(g);

    PathTreeBuilder builder;
    PathTree tree = builder.build(prepared);

    assert(tree.rootNode != -1);
    assert(!tree.nodes.empty());
    assert(tree.preorderNodes.size() == tree.nodes.size());

    bool foundNodeWithChildren = false;

    for (const PathNode& node : tree.nodes) {
        if (!node.children.empty()) {
            foundNodeWithChildren = true;
        }

        assert(!node.pathDarts.empty());
        assert(node.preorder >= 0);
        assert(node.subtreeEnd > node.preorder);
    }

    assert(foundNodeWithChildren);
}