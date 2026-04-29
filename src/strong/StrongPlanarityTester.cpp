#include "ht/strong/StrongPlanarityTester.hpp"

#include <stdexcept>
#include <utility>

namespace ht {

StrongPlanarityTester::StrongPlanarityTester(
    const PreparedPalmTree& prepared,
    const std::vector<int>& dfsNumber
) : P_(prepared), dfs_(dfsNumber) {
    if (static_cast<int>(dfs_.size()) != P_.n) {
        throw std::runtime_error("DFS number array size does not match prepared graph size.");
    }
}

const StrongPlanarityFailure& StrongPlanarityTester::failure() const {
    return failure_;
}

std::vector<int> StrongPlanarityTester::dequeToVector(const std::deque<int>& values) {
    return std::vector<int>(values.begin(), values.end());
}

void StrongPlanarityTester::clearFailure() {
    failure_ = StrongPlanarityFailure{};
    runRootTreeDart_ = -1;
}

void StrongPlanarityTester::recordFailure(
    StrongPlanarityFailureType type,
    int rootTreeDart,
    int currentDart,
    const CycleInfo& cycle,
    const Block& block,
    const std::vector<Block>& stack,
    const std::string& message
) {
    if (failure_.hasFailure()) {
        return;
    }

    failure_.type = type;
    failure_.rootTreeDart = runRootTreeDart_;
    failure_.cycleRootDart = rootTreeDart;
    failure_.currentDart = currentDart;

    failure_.x = cycle.x;
    failure_.y = cycle.y;
    failure_.w0 = cycle.w0;
    failure_.wk = cycle.wk;
    failure_.closingBackDart = cycle.closingBackDart;
    failure_.cycleSpineDarts = cycle.spineTreeDarts;

    failure_.blockLeftAttachments = dequeToVector(block.Latt);
    failure_.blockRightAttachments = dequeToVector(block.Ratt);
    failure_.blockLeftSegments = block.Lseg;
    failure_.blockRightSegments = block.Rseg;

    if (!stack.empty()) {
        const Block& top = stack.back();

        failure_.stackTopLeftAttachments = dequeToVector(top.Latt);
        failure_.stackTopRightAttachments = dequeToVector(top.Ratt);
        failure_.stackTopLeftSegments = top.Lseg;
        failure_.stackTopRightSegments = top.Rseg;
    }

    failure_.message = message;
}

bool StrongPlanarityTester::run(int rootTreeDart, std::vector<Side>& alpha) {
    clearFailure();

    if (rootTreeDart < 0 || rootTreeDart >= static_cast<int>(P_.darts.size())) {
        throw std::runtime_error("Invalid rootTreeDart in StrongPlanarityTester.");
    }

    runRootTreeDart_ = rootTreeDart;

    alpha.assign(P_.darts.size(), Side::Left);
    alpha[rootTreeDart] = Side::Left;

    std::vector<int> attachments;
    return stronglyPlanar(rootTreeDart, attachments, alpha);
}

StrongPlanarityTester::Block::Block(int segmentDart, const std::vector<int>& attachments) {
    Lseg.push_back(segmentDart);
    for (int a : attachments) {
        Latt.push_back(a);
    }
}

void StrongPlanarityTester::Block::flip() {
    std::swap(Latt, Ratt);
    std::swap(Lseg, Rseg);
}

bool StrongPlanarityTester::Block::leftInterlace(const std::vector<Block>& stack) const {
    if (Latt.empty()) {
        throw std::runtime_error("Invalid block: Latt should not be empty in leftInterlace.");
    }

    if (stack.empty() || stack.back().Latt.empty()) {
        return false;
    }

    return Latt.back() < stack.back().Latt.front();
}

bool StrongPlanarityTester::Block::rightInterlace(const std::vector<Block>& stack) const {
    if (Latt.empty()) {
        throw std::runtime_error("Invalid block: Latt should not be empty in rightInterlace.");
    }

    if (stack.empty() || stack.back().Ratt.empty()) {
        return false;
    }

    return Latt.back() < stack.back().Ratt.front();
}

void StrongPlanarityTester::Block::combine(const Block& other) {
    Latt.insert(Latt.end(), other.Latt.begin(), other.Latt.end());
    Ratt.insert(Ratt.end(), other.Ratt.begin(), other.Ratt.end());
    Lseg.insert(Lseg.end(), other.Lseg.begin(), other.Lseg.end());
    Rseg.insert(Rseg.end(), other.Rseg.begin(), other.Rseg.end());
}

bool StrongPlanarityTester::Block::clean(int dfsNumberToRemove, std::vector<Side>& alpha) {
    while (!Latt.empty() && Latt.front() == dfsNumberToRemove) {
        Latt.pop_front();
    }

    while (!Ratt.empty() && Ratt.front() == dfsNumberToRemove) {
        Ratt.pop_front();
    }

    if (!Latt.empty() || !Ratt.empty()) {
        return false;
    }

    for (int e : Lseg) {
        alpha[e] = Side::Left;
    }

    for (int e : Rseg) {
        alpha[e] = Side::Right;
    }

    return true;
}

void StrongPlanarityTester::Block::addToAttachments(
    std::vector<int>& attachments,
    int dfsW0,
    std::vector<Side>& alpha
) {
    if (!Ratt.empty() && Ratt.front() > dfsW0) {
        flip();
    }

    for (int a : Latt) {
        attachments.push_back(a);
    }

    for (int a : Ratt) {
        attachments.push_back(a);
    }

    for (int e : Lseg) {
        alpha[e] = Side::Left;
    }

    for (int e : Rseg) {
        alpha[e] = Side::Right;
    }
}

const Dart& StrongPlanarityTester::dart(int id) const {
    if (id < 0 || id >= static_cast<int>(P_.darts.size())) {
        throw std::runtime_error("Invalid dart id.");
    }

    return P_.darts[id];
}

int StrongPlanarityTester::firstOut(int v) const {
    if (v < 0 || v >= P_.n || P_.orderedOut[v].empty()) {
        throw std::runtime_error("Missing first outgoing dart in stronglyPlanar.");
    }

    return P_.orderedOut[v].front();
}

StrongPlanarityTester::CycleInfo StrongPlanarityTester::determineCycle(int e0) const {
    const Dart& start = dart(e0);

    if (!start.isTree) {
        throw std::runtime_error("stronglyPlanar expects a tree dart as e0.");
    }

    CycleInfo info;
    info.x = start.from;
    info.y = start.to;

    // The cycle spine starts with e0: x -> y.
    info.spineTreeDarts.push_back(e0);

    int e = firstOut(info.y);
    int wk = info.y;

    while (dfs_[dart(e).to] > dfs_[wk]) {
        if (!dart(e).isTree) {
            throw std::runtime_error("Expected tree dart while following cycle spine.");
        }

        info.spineTreeDarts.push_back(e);

        wk = dart(e).to;
        e = firstOut(wk);
    }

    info.wk = wk;
    info.w0 = dart(e).to;
    info.closingBackDart = e;

    if (!dart(e).isBack) {
        throw std::runtime_error("Cycle construction did not end in a back edge.");
    }

    return info;
}

bool StrongPlanarityTester::stronglyPlanar(
    int e0,
    std::vector<int>& attachments,
    std::vector<Side>& alpha
) {
    CycleInfo cycle = determineCycle(e0);

    std::vector<Block> stack;

    int w = cycle.wk;

    while (w != cycle.x) {
        const std::vector<int>& outgoing = P_.orderedOut[w];

        for (std::size_t i = 1; i < outgoing.size(); ++i) {
            const int e = outgoing[i];

            std::vector<int> childAttachments;

            if (dart(e).isTree) {
                if (!stronglyPlanar(e, childAttachments, alpha)) {
                    return false;
                }
            } else if (dart(e).isBack) {
                childAttachments.push_back(dfs_[dart(e).to]);
            } else {
                throw std::runtime_error("Outgoing dart is neither tree nor back dart.");
            }

            Block block(e, childAttachments);

            while (true) {
                if (block.leftInterlace(stack)) {
                    stack.back().flip();
                }

                if (block.leftInterlace(stack)) {
                    recordFailure(
                        StrongPlanarityFailureType::UnresolvableLeftInterlace,
                        e0,
                        e,
                        cycle,
                        block,
                        stack,
                        "Strong-planarity failed: unresolved left interlacing after stack flip."
                    );

                    return false;
                }

                if (block.rightInterlace(stack)) {
                    Block top = stack.back();
                    stack.pop_back();
                    block.combine(top);
                } else {
                    break;
                }
            }

            stack.push_back(block);
        }

        const int p = P_.parent[w];

        if (p == -1) {
            throw std::runtime_error("Unexpected root while walking back through cycle spine.");
        }

        while (!stack.empty() && stack.back().clean(dfs_[p], alpha)) {
            stack.pop_back();
        }

        w = p;
    }

    attachments.clear();

    while (!stack.empty()) {
        Block block = stack.back();
        stack.pop_back();

        const bool bothSidesAttachAboveW0 =
            !block.Latt.empty()
            && !block.Ratt.empty()
            && block.Latt.front() > dfs_[cycle.w0]
            && block.Ratt.front() > dfs_[cycle.w0];

        if (bothSidesAttachAboveW0) {
            recordFailure(
                StrongPlanarityFailureType::BothSidesAttachAboveW0,
                e0,
                -1,
                cycle,
                block,
                stack,
                "Strong-planarity failed: block has attachments on both sides above w0."
            );

            return false;
        }

        block.addToAttachments(attachments, dfs_[cycle.w0], alpha);
    }

    if (cycle.w0 != cycle.x) {
        attachments.push_back(dfs_[cycle.w0]);
    }

    return true;
}

} // namespace ht
