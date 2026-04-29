#pragma once

#include <string>
#include <vector>

#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class KuratowskiKernelSelector {
public:
    KuratowskiSubdivisionVerification select(
        const PreparedPalmTree& prepared,
        const std::vector<int>& candidateOriginalEdgeIds
    ) const;

private:
    struct EdgeRecord {
        int u = -1;
        int v = -1;
        int originalEdgeId = -1;
    };

    struct SkeletonEdge {
        int u = -1;
        int v = -1;
        std::vector<int> originalEdgeIds;
    };

    static constexpr int MaxSkeletonEdgesForBoundedSelection = 24;

    static int maxOriginalEdgeId(const PreparedPalmTree& prepared);

    static std::vector<EdgeRecord> buildSelectedEdges(
        const PreparedPalmTree& prepared,
        const std::vector<int>& candidateOriginalEdgeIds
    );

    static std::vector<std::vector<int>> buildIncidentEdges(
        int vertexCount,
        const std::vector<EdgeRecord>& edges,
        std::vector<int>& degree
    );

    static void pruneLeaves(
        const std::vector<EdgeRecord>& edges,
        const std::vector<std::vector<int>>& incidentEdges,
        std::vector<int>& degree,
        std::vector<char>& activeEdge
    );

    static bool buildSkeleton(
        int vertexCount,
        const std::vector<EdgeRecord>& edges,
        const std::vector<std::vector<int>>& incidentEdges,
        const std::vector<int>& degree,
        const std::vector<char>& activeEdge,
        std::vector<SkeletonEdge>& skeletonEdges,
        std::string& message
    );

    static bool trySubsetsOfSize(
        const PreparedPalmTree& prepared,
        const std::vector<SkeletonEdge>& skeletonEdges,
        int targetSize,
        KuratowskiSubdivisionVerification& selected
    );

    static bool chooseSubsetRecursive(
        const PreparedPalmTree& prepared,
        const std::vector<SkeletonEdge>& skeletonEdges,
        int targetSize,
        int index,
        std::vector<int>& chosen,
        KuratowskiSubdivisionVerification& selected
    );

    static std::vector<int> expandSkeletonSubset(
        const std::vector<SkeletonEdge>& skeletonEdges,
        const std::vector<int>& chosen
    );

    static int otherEndpoint(
        const EdgeRecord& edge,
        int vertex
    );
};

} // namespace ht