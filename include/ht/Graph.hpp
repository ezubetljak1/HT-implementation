#pragma once

#include <stdexcept>
#include <vector>

#include "ht/Types.hpp"

namespace ht {

class Graph {
public:
    explicit Graph(int vertexCount = 0);

    int addEdge(int u, int v);
    int addEdgeWithOriginalId(int u, int v, int originalEdgeId);

    int vertexCount() const;
    int edgeCount() const;

    const std::vector<AdjacentEdge>& adjacency(int v) const;
    const std::vector<Edge>& edges() const;
    const Edge& edge(int edgeId) const;

private:
    int vertexCount_ = 0;
    std::vector<Edge> edges_;
    std::vector<std::vector<AdjacentEdge>> adj_;

    void validateVertex(int v) const;
};

} // namespace ht
