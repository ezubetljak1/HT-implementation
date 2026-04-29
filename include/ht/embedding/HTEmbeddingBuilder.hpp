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
        int x = -1;
        int y = -1;
        int w0 = -1;
        int wk = -1;

        // spineVertices = x, y, ..., wk
        // We intentionally do not materialize the full stem w0 -> ... -> x.
        std::vector<int> spineVertices;

        // spineForwardDarts[i] is the tree dart spineVertices[i - 1] -> spineVertices[i].
        // spineForwardDarts[0] is unused and stays -1.
        std::vector<int> spineForwardDarts;

        // dart (wk, w0)
        int closingBackDart = -1;
    };

    const PreparedPalmTree& P_;
    std::vector<std::list<int>> rotation_;

    const Dart& dart(int id) const;
    int rev(int d) const;
    int firstOut(int v) const;

    CycleData buildCycleForTreeEdge(int e0) const;

    std::list<int> takeTailWithSource(std::list<int>& sourceList, int sourceVertex) const;
    std::list<int> takeHeadWithSource(std::list<int>& sourceList, int sourceVertex) const;

    void setRotation(int vertex, const std::list<int>& dartsInClockwiseOrder);
    ReturnedLists embedding(int e0, Side t);

    void validateInput() const;
};

} // namespace ht
