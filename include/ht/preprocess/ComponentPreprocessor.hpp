#pragma once

#include <unordered_map>
#include <vector>

#include "ht/Graph.hpp"
#include "ht/Types.hpp"

namespace ht {

struct ComponentContext {
    Graph localGraph = Graph(0);

    // local vertex id -> original vertex id
    std::vector<int> localToOriginal;

    // original vertex id -> local vertex id
    std::unordered_map<int, int> originalToLocal;
};

struct PreprocessedComponent {
    ComponentContext context;

    std::vector<int> number;
    std::vector<int> vertexByNumber;

    std::vector<int> parent;
    std::vector<int> parentEdgeId;
    std::vector<int> parentOriginalEdgeId;

    std::vector<std::vector<int>> children;

    std::vector<int> lowpt1;
    std::vector<int> lowpt2;
    std::vector<LowpointInfo> lowpointInfo;

    // Active HT orientations sorted by phi value.
    std::vector<std::vector<Arc>> orderedOut;
};

class ComponentPreprocessor {
public:
    PreprocessedComponent preprocess(const Component& component);

private:
    PreprocessedComponent result_;
    std::vector<std::vector<Arc>> activeOut_;
    int dfsCounter_ = 0;

    ComponentContext buildLocalGraph(const Component& component);
    int getOrCreateLocalVertex(ComponentContext& context, int originalVertex);

    void initializeForDfs(int vertexCount);
    void lowptDfs(int v, int parentVertex, int parentEdgeId);

    void addLowptCandidate(
        int v,
        int candidateNumber,
        int candidateVertex,
        int edgeId,
        int originalEdgeId
    );

    int computePhi(const Arc& arc) const;
    void assignPhiValues();
    void bucketSortOutgoingArcs();
};

} // namespace ht
