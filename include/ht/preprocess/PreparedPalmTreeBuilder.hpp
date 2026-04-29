#pragma once

#include <unordered_map>

#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class PreparedPalmTreeBuilder {
public:
    PreparedPalmTree build(const PreprocessedComponent& component) const;

private:
    long long directedKey(int from, int to) const;

    void validateInput(const PreprocessedComponent& component) const;

    void addDartPair(
        PreparedPalmTree& prepared,
        const Edge& edge,
        std::unordered_map<long long, int>& dartByDirectedEdge
    ) const;

    void createDarts(
        PreparedPalmTree& prepared,
        const PreprocessedComponent& component,
        std::unordered_map<long long, int>& dartByDirectedEdge
    ) const;

    void markDartTypes(PreparedPalmTree& prepared) const;

    void fillOrderedOutDarts(
        PreparedPalmTree& prepared,
        const PreprocessedComponent& component,
        const std::unordered_map<long long, int>& dartByDirectedEdge
    ) const;

    int findRoot(const PreparedPalmTree& prepared) const;
    void setRootTreeDart(PreparedPalmTree& prepared) const;
};

} // namespace ht
