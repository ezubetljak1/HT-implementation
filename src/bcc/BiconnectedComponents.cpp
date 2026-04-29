#include "ht/bcc/BiconnectedComponents.hpp"

#include <algorithm>
#include <stdexcept>

namespace ht {

Components BiconnectedComponentsFinder::find(const Graph& graph) {
    graph_ = &graph;
    timer_ = 0;

    const int n = graph.vertexCount();

    disc_.assign(n, 0);
    low_.assign(n, 0);
    parent_.assign(n, -1);
    parentEdge_.assign(n, -1);

    edgeStack_.clear();
    components_.clear();

    for (int v = 0; v < n; ++v) {
        if (disc_[v] == 0) {
            dfs(v);

            // Defensive cleanup for disconnected graphs.
            if (!edgeStack_.empty()) {
                Component component;
                while (!edgeStack_.empty()) {
                    component.push_back(edgeStack_.back());
                    edgeStack_.pop_back();
                }
                components_.push_back(component);
            }
        }
    }

    return components_;
}

void BiconnectedComponentsFinder::dfs(int v) {
    disc_[v] = low_[v] = ++timer_;

    for (const AdjacentEdge& adj : graph_->adjacency(v)) {
        const int w = adj.to;
        const int edgeId = adj.edgeId;

        if (disc_[w] == 0) {
            parent_[w] = v;
            parentEdge_[w] = edgeId;

            edgeStack_.push_back(graph_->edge(edgeId));

            dfs(w);

            low_[v] = std::min(low_[v], low_[w]);

            if (low_[w] >= disc_[v]) {
                popComponentUntil(edgeId);
            }
        } else if (edgeId != parentEdge_[v] && disc_[w] < disc_[v]) {
            low_[v] = std::min(low_[v], disc_[w]);
            edgeStack_.push_back(graph_->edge(edgeId));
        }
    }
}

void BiconnectedComponentsFinder::popComponentUntil(int edgeId) {
    Component component;

    while (true) {
        if (edgeStack_.empty()) {
            throw std::runtime_error("BCC edge stack unexpectedly empty.");
        }

        Edge edge = edgeStack_.back();
        edgeStack_.pop_back();
        component.push_back(edge);

        if (edge.id == edgeId) {
            break;
        }
    }

    components_.push_back(component);
}

} // namespace ht
