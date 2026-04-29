#include "ht/Graph.hpp"

namespace ht {

Graph::Graph(int vertexCount)
    : vertexCount_(vertexCount), adj_(vertexCount) {
    if (vertexCount < 0) {
        throw std::invalid_argument("Graph vertex count cannot be negative.");
    }
}

int Graph::addEdge(int u, int v) {
    int id = static_cast<int>(edges_.size());
    return addEdgeWithOriginalId(u, v, id);
}

int Graph::addEdgeWithOriginalId(int u, int v, int originalEdgeId) {
    validateVertex(u);
    validateVertex(v);

    if (u == v) {
        throw std::invalid_argument("Self-loops are not supported.");
    }

    // Duplicate checking is intentionally not performed here.
    // The intended input for the final HT pipeline is a simple graph.
    // Adding duplicate checking by scanning adjacency lists would harm the clean O(V + E) model.
    int id = static_cast<int>(edges_.size());
    edges_.push_back(Edge{id, originalEdgeId, u, v});

    adj_[u].push_back(AdjacentEdge{v, id});
    adj_[v].push_back(AdjacentEdge{u, id});

    return id;
}

int Graph::vertexCount() const {
    return vertexCount_;
}

int Graph::edgeCount() const {
    return static_cast<int>(edges_.size());
}

const std::vector<AdjacentEdge>& Graph::adjacency(int v) const {
    validateVertex(v);
    return adj_[v];
}

const std::vector<Edge>& Graph::edges() const {
    return edges_;
}

const Edge& Graph::edge(int edgeId) const {
    if (edgeId < 0 || edgeId >= static_cast<int>(edges_.size())) {
        throw std::out_of_range("Edge id is outside graph edge range.");
    }

    return edges_[edgeId];
}

void Graph::validateVertex(int v) const {
    if (v < 0 || v >= vertexCount_) {
        throw std::out_of_range("Vertex is outside graph vertex range.");
    }
}

} // namespace ht
