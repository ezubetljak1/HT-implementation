#pragma once

#include <vector>

#include "ht/Graph.hpp"
#include "ht/Types.hpp"

namespace ht {

class BiconnectedComponentsFinder {
public:
    Components find(const Graph& graph);

private:
    const Graph* graph_ = nullptr;
    int timer_ = 0;

    std::vector<int> disc_;
    std::vector<int> low_;
    std::vector<int> parent_;
    std::vector<int> parentEdge_;

    std::vector<Edge> edgeStack_;
    Components components_;

    void dfs(int v);
    void popComponentUntil(int edgeId);
};

} // namespace ht
