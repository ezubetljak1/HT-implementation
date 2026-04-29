#include "ht/preprocess/ComponentPreprocessor.hpp"

#include <stdexcept>

namespace ht {

PreprocessedComponent ComponentPreprocessor::preprocess(const Component& component) {
    result_ = PreprocessedComponent();
    result_.context = buildLocalGraph(component);

    const int vertexCount = result_.context.localGraph.vertexCount();
    initializeForDfs(vertexCount);

    for (int v = 0; v < vertexCount; ++v) {
        if (result_.number[v] == 0) {
            lowptDfs(v, -1, -1);
        }
    }

    assignPhiValues();
    bucketSortOutgoingArcs();

    return result_;
}

ComponentContext ComponentPreprocessor::buildLocalGraph(const Component& component) {
    ComponentContext context;

    for (const Edge& edge : component) {
        getOrCreateLocalVertex(context, edge.u);
        getOrCreateLocalVertex(context, edge.v);
    }

    context.localGraph = Graph(static_cast<int>(context.localToOriginal.size()));

    for (const Edge& edge : component) {
        const int localU = context.originalToLocal.at(edge.u);
        const int localV = context.originalToLocal.at(edge.v);
        context.localGraph.addEdgeWithOriginalId(localU, localV, edge.originalId);
    }

    return context;
}

int ComponentPreprocessor::getOrCreateLocalVertex(ComponentContext& context, int originalVertex) {
    auto it = context.originalToLocal.find(originalVertex);
    if (it != context.originalToLocal.end()) {
        return it->second;
    }

    const int localVertex = static_cast<int>(context.localToOriginal.size());
    context.originalToLocal[originalVertex] = localVertex;
    context.localToOriginal.push_back(originalVertex);

    return localVertex;
}

void ComponentPreprocessor::initializeForDfs(int vertexCount) {
    result_.number.assign(vertexCount, 0);
    result_.vertexByNumber.assign(vertexCount + 1, -1);

    result_.parent.assign(vertexCount, -1);
    result_.parentEdgeId.assign(vertexCount, -1);
    result_.parentOriginalEdgeId.assign(vertexCount, -1);

    result_.children.assign(vertexCount, {});

    result_.lowpt1.assign(vertexCount, 0);
    result_.lowpt2.assign(vertexCount, 0);
    result_.lowpointInfo.assign(vertexCount, {});

    activeOut_.assign(vertexCount, {});
    result_.orderedOut.assign(vertexCount, {});

    dfsCounter_ = 0;
}

void ComponentPreprocessor::lowptDfs(int v, int parentVertex, int parentEdgeId) {
    result_.number[v] = ++dfsCounter_;
    result_.vertexByNumber[result_.number[v]] = v;

    result_.parent[v] = parentVertex;
    result_.parentEdgeId[v] = parentEdgeId;

    result_.lowpt1[v] = result_.number[v];
    result_.lowpt2[v] = result_.number[v];

    result_.lowpointInfo[v].lowpt1 = result_.number[v];
    result_.lowpointInfo[v].lowpt2 = result_.number[v];
    result_.lowpointInfo[v].lowpt1Vertex = v;
    result_.lowpointInfo[v].lowpt2Vertex = v;

    for (const AdjacentEdge& adj : result_.context.localGraph.adjacency(v)) {
        const int w = adj.to;
        const int edgeId = adj.edgeId;
        const Edge& edge = result_.context.localGraph.edge(edgeId);

        if (result_.number[w] == 0) {
            result_.children[v].push_back(w);
            result_.parentOriginalEdgeId[w] = edge.originalId;

            activeOut_[v].push_back(
                Arc(edgeId, edge.originalId, v, w, ArcType::TreeArc)
            );

            lowptDfs(w, v, edgeId);

            addLowptCandidate(
                v,
                result_.lowpt1[w],
                result_.lowpointInfo[w].lowpt1Vertex,
                result_.lowpointInfo[w].lowpt1EdgeId,
                result_.lowpointInfo[w].lowpt1OriginalEdgeId
            );

            addLowptCandidate(
                v,
                result_.lowpt2[w],
                result_.lowpointInfo[w].lowpt2Vertex,
                result_.lowpointInfo[w].lowpt2EdgeId,
                result_.lowpointInfo[w].lowpt2OriginalEdgeId
            );
        } else if (edgeId != parentEdgeId && result_.number[w] < result_.number[v]) {
            activeOut_[v].push_back(
                Arc(edgeId, edge.originalId, v, w, ArcType::Frond)
            );

            addLowptCandidate(
                v,
                result_.number[w],
                w,
                edgeId,
                edge.originalId
            );
        }
    }
}

void ComponentPreprocessor::addLowptCandidate(
    int v,
    int candidateNumber,
    int candidateVertex,
    int edgeId,
    int originalEdgeId
) {
    if (candidateNumber < result_.lowpt1[v]) {
        result_.lowpt2[v] = result_.lowpt1[v];
        result_.lowpointInfo[v].lowpt2 = result_.lowpointInfo[v].lowpt1;
        result_.lowpointInfo[v].lowpt2Vertex = result_.lowpointInfo[v].lowpt1Vertex;
        result_.lowpointInfo[v].lowpt2EdgeId = result_.lowpointInfo[v].lowpt1EdgeId;
        result_.lowpointInfo[v].lowpt2OriginalEdgeId = result_.lowpointInfo[v].lowpt1OriginalEdgeId;

        result_.lowpt1[v] = candidateNumber;
        result_.lowpointInfo[v].lowpt1 = candidateNumber;
        result_.lowpointInfo[v].lowpt1Vertex = candidateVertex;
        result_.lowpointInfo[v].lowpt1EdgeId = edgeId;
        result_.lowpointInfo[v].lowpt1OriginalEdgeId = originalEdgeId;
    } else if (candidateNumber > result_.lowpt1[v] && candidateNumber < result_.lowpt2[v]) {
        result_.lowpt2[v] = candidateNumber;
        result_.lowpointInfo[v].lowpt2 = candidateNumber;
        result_.lowpointInfo[v].lowpt2Vertex = candidateVertex;
        result_.lowpointInfo[v].lowpt2EdgeId = edgeId;
        result_.lowpointInfo[v].lowpt2OriginalEdgeId = originalEdgeId;
    }
}

int ComponentPreprocessor::computePhi(const Arc& arc) const {
    const int v = arc.from;
    const int w = arc.to;

    if (arc.type == ArcType::Frond) {
        return 2 * result_.number[w];
    }

    if (arc.type == ArcType::TreeArc) {
        if (result_.lowpt2[w] >= result_.number[v]) {
            return 2 * result_.lowpt1[w];
        }

        return 2 * result_.lowpt1[w] + 1;
    }

    throw std::runtime_error("Unknown arc type.");
}

void ComponentPreprocessor::assignPhiValues() {
    for (std::vector<Arc>& arcs : activeOut_) {
        for (Arc& arc : arcs) {
            arc.phi = computePhi(arc);
        }
    }
}

void ComponentPreprocessor::bucketSortOutgoingArcs() {
    const int vertexCount = result_.context.localGraph.vertexCount();
    const int maxPhiValue = 2 * vertexCount + 1;

    std::vector<std::vector<Arc>> buckets(maxPhiValue + 1);

    for (int v = 0; v < vertexCount; ++v) {
        for (const Arc& arc : activeOut_[v]) {
            if (arc.phi < 0 || arc.phi > maxPhiValue) {
                throw std::runtime_error("Invalid phi value.");
            }

            buckets[arc.phi].push_back(arc);
        }
    }

    result_.orderedOut.assign(vertexCount, {});

    for (int phi = 0; phi <= maxPhiValue; ++phi) {
        for (const Arc& arc : buckets[phi]) {
            result_.orderedOut[arc.from].push_back(arc);
        }
    }
}

} // namespace ht
