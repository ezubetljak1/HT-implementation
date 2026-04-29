#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"

#include <cstdint>
#include <stdexcept>

namespace ht {

PreparedPalmTree PreparedPalmTreeBuilder::build(const PreprocessedComponent& component) const {
    validateInput(component);

    PreparedPalmTree prepared;

    prepared.n = component.context.localGraph.vertexCount();
    prepared.edgeCount = component.context.localGraph.edgeCount();
    prepared.localToOriginal = component.context.localToOriginal;
    prepared.number = component.number;
    prepared.parent = component.parent;

    prepared.outAll.assign(prepared.n, {});
    prepared.orderedOut.assign(prepared.n, {});

    std::unordered_map<long long, int> dartByDirectedEdge;

    createDarts(prepared, component, dartByDirectedEdge);
    markDartTypes(prepared);
    fillOrderedOutDarts(prepared, component, dartByDirectedEdge);
    setRootTreeDart(prepared);

    prepared.alpha.assign(prepared.darts.size(), Side::Left);

    return prepared;
}

long long PreparedPalmTreeBuilder::directedKey(int from, int to) const {
    return (static_cast<long long>(static_cast<unsigned int>(from)) << 32)
        | static_cast<unsigned int>(to);
}

void PreparedPalmTreeBuilder::validateInput(const PreprocessedComponent& component) const {
    const int n = component.context.localGraph.vertexCount();

    if (static_cast<int>(component.number.size()) != n) {
        throw std::runtime_error("number size does not match local graph vertex count.");
    }

    if (static_cast<int>(component.orderedOut.size()) != n) {
        throw std::runtime_error("orderedOut size does not match local graph vertex count.");
    }

    if (static_cast<int>(component.parent.size()) != n) {
        throw std::runtime_error("parent size does not match local graph vertex count.");
    }

    if (static_cast<int>(component.context.localToOriginal.size()) != n) {
        throw std::runtime_error("localToOriginal size does not match local graph vertex count.");
    }
}

void PreparedPalmTreeBuilder::addDartPair(
    PreparedPalmTree& prepared,
    const Edge& edge,
    std::unordered_map<long long, int>& dartByDirectedEdge
) const {
    const int firstId = static_cast<int>(prepared.darts.size());
    const int secondId = firstId + 1;

    Dart first;
    first.id = firstId;
    first.edgeId = edge.id;
    first.originalEdgeId = edge.originalId;
    first.from = edge.u;
    first.to = edge.v;
    first.rev = secondId;

    Dart second;
    second.id = secondId;
    second.edgeId = edge.id;
    second.originalEdgeId = edge.originalId;
    second.from = edge.v;
    second.to = edge.u;
    second.rev = firstId;

    prepared.darts.push_back(first);
    prepared.darts.push_back(second);

    prepared.outAll[edge.u].push_back(firstId);
    prepared.outAll[edge.v].push_back(secondId);

    dartByDirectedEdge[directedKey(edge.u, edge.v)] = firstId;
    dartByDirectedEdge[directedKey(edge.v, edge.u)] = secondId;
}

void PreparedPalmTreeBuilder::createDarts(
    PreparedPalmTree& prepared,
    const PreprocessedComponent& component,
    std::unordered_map<long long, int>& dartByDirectedEdge
) const {
    for (const Edge& edge : component.context.localGraph.edges()) {
        addDartPair(prepared, edge, dartByDirectedEdge);
    }
}

void PreparedPalmTreeBuilder::markDartTypes(PreparedPalmTree& prepared) const {
    for (Dart& dart : prepared.darts) {
        const int from = dart.from;
        const int to = dart.to;

        if (prepared.parent[to] == from) {
            dart.isTree = true;
        } else if (prepared.number[to] < prepared.number[from] && prepared.parent[from] != to) {
            dart.isBack = true;
        }
    }
}

void PreparedPalmTreeBuilder::fillOrderedOutDarts(
    PreparedPalmTree& prepared,
    const PreprocessedComponent& component,
    const std::unordered_map<long long, int>& dartByDirectedEdge
) const {
    for (int v = 0; v < prepared.n; ++v) {
        for (const Arc& arc : component.orderedOut[v]) {
            if (arc.from != v) {
                throw std::runtime_error("Arc is stored under the wrong source vertex.");
            }

            auto it = dartByDirectedEdge.find(directedKey(arc.from, arc.to));
            if (it == dartByDirectedEdge.end()) {
                throw std::runtime_error("Could not map ordered arc to dart.");
            }

            const int dartId = it->second;
            Dart& dart = prepared.darts[dartId];

            if (arc.type == ArcType::TreeArc && !dart.isTree) {
                throw std::runtime_error("Tree arc does not map to a tree dart.");
            }

            if (arc.type == ArcType::Frond && !dart.isBack) {
                throw std::runtime_error("Frond arc does not map to a back dart.");
            }

            dart.phi = arc.phi;
            prepared.orderedOut[v].push_back(dartId);
        }
    }
}

int PreparedPalmTreeBuilder::findRoot(const PreparedPalmTree& prepared) const {
    for (int v = 0; v < prepared.n; ++v) {
        if (prepared.parent[v] == -1) {
            return v;
        }
    }

    throw std::runtime_error("No DFS root found.");
}

void PreparedPalmTreeBuilder::setRootTreeDart(PreparedPalmTree& prepared) const {
    if (prepared.n <= 1) {
        prepared.rootTreeDart = -1;
        return;
    }

    const int root = findRoot(prepared);

    for (int dartId : prepared.orderedOut[root]) {
        if (prepared.darts[dartId].isTree) {
            prepared.rootTreeDart = dartId;
            return;
        }
    }

    // A component with one edge or isolated vertex may not have a usable HT root tree dart.
    prepared.rootTreeDart = -1;
}

} // namespace ht
