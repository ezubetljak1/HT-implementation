#pragma once

#include <list>
#include <vector>

#include "ht/embedding/Embedding.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class HTEmbeddingBuilder {
public:
    explicit HTEmbeddingBuilder(const PreparedPalmTree& prepared);

    Embedding run();

private:
    struct ReturnedLists {
        std::list<int> T;
        std::list<int> A;
    };

    struct CycleData {
        // cycleVertices = w0, w1, ..., wr, wr+1, ..., wk
        std::vector<int> cycleVertices;
        int r = -1;
        int k = -1;

        // dart (wk, w0)
        int closingBackDart = -1;
    };

    const PreparedPalmTree& P_;
    std::vector<std::list<int>> rotation_;

    const Dart& dart(int id) const;
    int rev(int d) const;
    int firstOut(int v) const;
    int findDart(int from, int to) const;

    std::vector<int> treePathFromAncestorToVertex(int ancestor, int x) const;
    CycleData buildCycleForTreeEdge(int e0) const;

    std::list<int> takeTailWithSource(std::list<int>& sourceList, int sourceVertex) const;
    std::list<int> takeHeadWithSource(std::list<int>& sourceList, int sourceVertex) const;

    void setRotation(int vertex, const std::list<int>& dartsInClockwiseOrder);
    ReturnedLists embedding(int e0, Side t);

    void validateInput() const;
};

} // namespace ht
