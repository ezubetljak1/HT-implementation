#pragma once

#include <deque>
#include <vector>

#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class StrongPlanarityTester {
public:
    StrongPlanarityTester(
        const PreparedPalmTree& prepared,
        const std::vector<int>& dfsNumber
    );

    bool run(int rootTreeDart, std::vector<Side>& alpha);

private:
    struct Block {
        std::deque<int> Latt;
        std::deque<int> Ratt;
        std::vector<int> Lseg;
        std::vector<int> Rseg;

        Block() = default;
        Block(int segmentDart, const std::vector<int>& attachments);

        void flip();

        bool leftInterlace(const std::vector<Block>& stack) const;
        bool rightInterlace(const std::vector<Block>& stack) const;

        void combine(const Block& other);
        bool clean(int dfsNumberToRemove, std::vector<Side>& alpha);

        void addToAttachments(
            std::vector<int>& attachments,
            int dfsW0,
            std::vector<Side>& alpha
        );
    };

    struct CycleInfo {
        int x = -1;
        int y = -1;
        int wk = -1;
        int w0 = -1;
        int closingBackDart = -1;
    };

    const PreparedPalmTree& P_;
    const std::vector<int>& dfs_;

    const Dart& dart(int id) const;
    int firstOut(int v) const;

    CycleInfo determineCycle(int e0) const;

    bool stronglyPlanar(
        int e0,
        std::vector<int>& attachments,
        std::vector<Side>& alpha
    );
};

} // namespace ht
