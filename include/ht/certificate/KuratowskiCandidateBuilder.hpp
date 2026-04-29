#pragma once

#include <vector>

#include "ht/certificate/PathTreeQueries.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

namespace ht {

class KuratowskiCandidateBuilder {
public:
    std::vector<int> buildOriginalEdgeCandidate(
        const PreparedPalmTree& prepared,
        const StrongPlanarityFailure& failure
    ) const;

private:
    static int maxOriginalEdgeId(const PreparedPalmTree& prepared);

    static void addUniqueOriginalEdgeFromDart(
        const PreparedPalmTree& prepared,
        int dartId,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );

    static void addUniqueOriginalEdgesFromDarts(
        const PreparedPalmTree& prepared,
        const std::vector<int>& dartIds,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );

    static void addCycleForDart(
        const PreparedPalmTree& prepared,
        const PathTreeQueries& queries,
        int dartId,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );

    static void addSegmentForDart(
        const PreparedPalmTree& prepared,
        const PathTreeQueries& queries,
        int dartId,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );

    static void addSegmentsForDarts(
        const PreparedPalmTree& prepared,
        const PathTreeQueries& queries,
        const std::vector<int>& dartIds,
        std::vector<char>& seen,
        std::vector<int>& originalEdgeIds
    );
};

} // namespace ht