#include "ht/embedding/HTEmbeddingBuilder.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace ht {

HTEmbeddingBuilder::HTEmbeddingBuilder(const PreparedPalmTree& prepared)
    : P_(prepared), 
    rotation_(prepared.n) {
    validateInput();
}

Embedding HTEmbeddingBuilder::run() {
    const int e0 = P_.rootTreeDart;
    const int root = dart(e0).from;

    ReturnedLists rootLists = embedding(e0, Side::Left);

    // Root call has no parent call that consumes the returned non-standard stem representation.
    std::list<int> rootRotation;
    rootRotation.splice(rootRotation.end(), rootLists.T);
    rootRotation.splice(rootRotation.end(), rootLists.A);
    setRotation(root, rootRotation);

    Embedding result;
    result.rotationDarts.resize(P_.n);
    result.rotationNeighbors.resize(P_.n);
    result.rotationOriginalEdgeIds.resize(P_.n);
    result.rotationOriginalNeighbors.resize(P_.n);

    for (int v = 0; v < P_.n; ++v) {
        for (int d : rotation_[v]) {
            const Dart& current = dart(d);

            result.rotationDarts[v].push_back(d);
            result.rotationNeighbors[v].push_back(current.to);

            result.rotationOriginalEdgeIds[v].push_back(current.originalEdgeId);

            if (!P_.localToOriginal.empty()) {
                result.rotationOriginalNeighbors[v].push_back(P_.localToOriginal[current.to]);
            } else {
                result.rotationOriginalNeighbors[v].push_back(current.to);
            }
        }
    }

    return result;
}

const Dart& HTEmbeddingBuilder::dart(int id) const {
    if (id < 0 || id >= static_cast<int>(P_.darts.size())) {
        throw std::runtime_error("Invalid dart id.");
    }

    return P_.darts[id];
}

int HTEmbeddingBuilder::rev(int d) const {
    return dart(d).rev;
}

int HTEmbeddingBuilder::firstOut(int v) const {
    if (v < 0 || v >= P_.n || P_.orderedOut[v].empty()) {
        throw std::runtime_error("Missing first outgoing dart while constructing spine.");
    }

    return P_.orderedOut[v].front();
}

HTEmbeddingBuilder::CycleData HTEmbeddingBuilder::buildCycleForTreeEdge(int e0) const {
    if (!dart(e0).isTree) {
        throw std::runtime_error("embedding(e0, t) expects e0 to be a tree dart.");
    }

    CycleData c;

    c.x = dart(e0).from;
    c.y = dart(e0).to;

    c.spineVertices.push_back(c.x);
    c.spineForwardDarts.push_back(-1);

    c.spineVertices.push_back(c.y);
    c.spineForwardDarts.push_back(e0);

    int current = c.y;

    while (true) {
        const int first = firstOut(current);

        if (dart(first).isBack) {
            c.wk = current;
            c.w0 = dart(first).to;
            c.closingBackDart = first;
            break;
        }

        if (!dart(first).isTree) {
            throw std::runtime_error("First outgoing dart is neither tree nor back dart.");
        }

        const int next = dart(first).to;

        c.spineVertices.push_back(next);
        c.spineForwardDarts.push_back(first);

        current = next;
    }

    if (c.w0 == -1 || c.wk == -1 || c.closingBackDart == -1) {
        throw std::runtime_error("Failed to construct HT cycle data.");
    }

    if (c.spineVertices.size() != c.spineForwardDarts.size()) {
        throw std::runtime_error("Spine vertices and spine darts have inconsistent sizes.");
    }

    return c;
}

std::list<int> HTEmbeddingBuilder::takeTailWithSource(
    std::list<int>& sourceList,
    int sourceVertex
) const {
    std::list<int> taken;

    while (!sourceList.empty() && dart(sourceList.back()).from == sourceVertex) {
        taken.push_front(sourceList.back());
        sourceList.pop_back();
    }

    return taken;
}

std::list<int> HTEmbeddingBuilder::takeHeadWithSource(
    std::list<int>& sourceList,
    int sourceVertex
) const {
    std::list<int> taken;

    while (!sourceList.empty() && dart(sourceList.front()).from == sourceVertex) {
        taken.push_back(sourceList.front());
        sourceList.pop_front();
    }

    return taken;
}

void HTEmbeddingBuilder::setRotation(
    int vertex,
    const std::list<int>& dartsInClockwiseOrder
) {
    if (vertex < 0 || vertex >= P_.n) {
        throw std::runtime_error("Invalid vertex in setRotation.");
    }

    if (!rotation_[vertex].empty()) {
        throw std::runtime_error(
            "Rotation for vertex " + std::to_string(vertex) + " was assigned twice."
        );
    }

    rotation_[vertex] = dartsInClockwiseOrder;
}

HTEmbeddingBuilder::ReturnedLists HTEmbeddingBuilder::embedding(int e0, Side t) {
    CycleData C = buildCycleForTreeEdge(e0);

    std::list<int> AL;
    std::list<int> AR;

    // T starts with the closing back edge (wk, w0).
    std::list<int> T;
    T.push_back(C.closingBackDart);

    // Walk down the spine: wk, ..., y.
    // spineVertices = x, y, ..., wk, so j goes from last index down to 1.
    for (int j = static_cast<int>(C.spineVertices.size()) - 1; j >= 1; --j) {
        const int wj = C.spineVertices[j];
        const auto& outgoing = P_.orderedOut[wj];

        if (outgoing.empty()) {
            throw std::runtime_error("Spine vertex has no ordered outgoing darts.");
        }

        // Process all emanating edges from wj except the first one.
        for (std::size_t pos = 1; pos < outgoing.size(); ++pos) {
            const int e = outgoing[pos];

            if (e < 0 || e >= static_cast<int>(P_.alpha.size())) {
                throw std::runtime_error("Missing alpha entry for emanating dart.");
            }

            const Side alphaE = P_.alpha[e];

            ReturnedLists child;

            if (dart(e).isTree) {
                child = embedding(e, xorSide(t, alphaE));
            } else if (dart(e).isBack) {
                child.T.push_back(e);
                child.A.push_back(rev(e));
            } else {
                throw std::runtime_error("Emanating dart is neither tree nor back dart.");
            }

            if (t == alphaE) {
                T.splice(T.begin(), child.T);
                AL.splice(AL.end(), child.A);
            } else {
                T.splice(T.end(), child.T);
                AR.splice(AR.begin(), child.A);
            }
        }

        const int wPrev = C.spineVertices[j - 1];

        const int forwardDart = C.spineForwardDarts[j];

        if (forwardDart == -1) {
            throw std::runtime_error("Missing forward dart for cycle edge.");
        }

        std::list<int> wjRotation;
        wjRotation.push_back(rev(forwardDart));
        wjRotation.insert(wjRotation.end(), T.begin(), T.end());
        setRotation(wj, wjRotation);

        const int nextVertex = wPrev;

        std::list<int> leftIncident = takeTailWithSource(AL, nextVertex);
        std::list<int> rightIncident = takeHeadWithSource(AR, nextVertex);

        T.clear();
        T.splice(T.end(), leftIncident);
        T.push_back(forwardDart);
        T.splice(T.end(), rightIncident);
    }

    ReturnedLists result;
    result.T.splice(result.T.end(), T);
    result.A.splice(result.A.end(), AR);
    result.A.push_back(rev(C.closingBackDart));
    result.A.splice(result.A.end(), AL);

    return result;
}

void HTEmbeddingBuilder::validateInput() const {
    if (P_.n <= 0) {
        throw std::runtime_error("PreparedPalmTree has no vertices.");
    }

    if (P_.rootTreeDart < 0 || P_.rootTreeDart >= static_cast<int>(P_.darts.size())) {
        throw std::runtime_error("Invalid rootTreeDart.");
    }

    if (!dart(P_.rootTreeDart).isTree) {
        throw std::runtime_error("rootTreeDart must be a tree dart.");
    }

    if (static_cast<int>(P_.outAll.size()) != P_.n
        || static_cast<int>(P_.orderedOut.size()) != P_.n
        || static_cast<int>(P_.parent.size()) != P_.n) {
        throw std::runtime_error("PreparedPalmTree arrays have inconsistent sizes.");
    }

    if (P_.alpha.size() != P_.darts.size()) {
        throw std::runtime_error("alpha must have one entry per dart.");
    }

    for (const Dart& d : P_.darts) {
        if (d.id < 0 || d.id >= static_cast<int>(P_.darts.size())) {
            throw std::runtime_error("Dart has invalid id.");
        }

        if (d.rev < 0 || d.rev >= static_cast<int>(P_.darts.size())) {
            throw std::runtime_error("Dart has invalid reversal.");
        }

        if (P_.darts[d.rev].rev != d.id) {
            throw std::runtime_error("Dart reversal relation is not symmetric.");
        }

        if (d.from < 0 || d.from >= P_.n || d.to < 0 || d.to >= P_.n) {
            throw std::runtime_error("Dart endpoint is outside vertex range.");
        }
    }
}

} // namespace ht
