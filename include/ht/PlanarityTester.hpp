#pragma once

#include "ht/Graph.hpp"
#include "ht/PlanarityResult.hpp"

namespace ht {

class PlanarityTester {
public:
    PlanarityResult test(const Graph& graph, bool buildEmbedding = true) const;
};

} // namespace ht
