#pragma once

#include <deque>
#include <vector>
#include <string>

#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

enum class StrongPlanarityFailureType {
    None,
    UnresolvableLeftInterlace,
    BothSidesAttachAboveW0
};

struct StrongPlanarityFailure {
    StrongPlanarityFailureType type = StrongPlanarityFailureType::None;

    int rootTreeDart = -1;
    int cycleRootDart = -1;
    int currentDart = -1;

    int x = -1;
    int y = -1;
    int w0 = -1;
    int wk = -1;
    int closingBackDart = -1;

    std::vector<int> cycleSpineDarts;
    std::vector<int> cycleStemDarts;
    std::vector<int> cycleTreeDarts;
    std::vector<int> cycleEmanatingDarts;
    std::vector<int> cycleRootEmanatingDarts;

    std::vector<int> blockLeftAttachments;
    std::vector<int> blockRightAttachments;
    std::vector<int> blockLeftSegments;
    std::vector<int> blockRightSegments;

    std::vector<int> stackTopLeftAttachments;
    std::vector<int> stackTopRightAttachments;
    std::vector<int> stackTopLeftSegments;
    std::vector<int> stackTopRightSegments;

    std::string message;

    bool hasFailure() const {
        return type != StrongPlanarityFailureType::None;
    }
};

class StrongPlanarityTester {
public:
    StrongPlanarityTester(
        const PreparedPalmTree& prepared,
        const std::vector<int>& dfsNumber
    );

    bool run(int rootTreeDart, std::vector<Side>& alpha);

    const StrongPlanarityFailure& failure() const;

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
        // Tree darts on the cycle spine: x -> y -> ... -> wk.
        std::vector<int> spineTreeDarts;
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

    StrongPlanarityFailure failure_;
    int runRootTreeDart_ = -1;

    static std::vector<int> dequeToVector(const std::deque<int>& values);

    void clearFailure();

    void recordFailure(
        StrongPlanarityFailureType type,
        int rootTreeDart,
        int currentDart,
        const CycleInfo& cycle,
        const std::vector<int>& cycleEmanatingDarts,
        const Block& block,
        const std::vector<Block>& stack,
        const std::string& message
    );

    std::vector<int> treeDartFromParent_;

    void buildTreeDartFromParentIndex();

    std::vector<int> treePathDartsFromAncestorToDescendant(
        int ancestor,
        int descendant
    ) const;

};

} // namespace ht
