#pragma once

#include <string>
#include <vector>

#include "ht/certificate/KuratowskiCertificate.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

struct KuratowskiSubdivisionVerification {
    bool valid = false;

    KuratowskiType type = KuratowskiType::Unknown;

    // Cleaned original edge IDs after optional leaf pruning.
    std::vector<int> originalEdgeIds;

    int activeVertexCount = 0;
    int activeEdgeCount = 0;
    int branchVertexCount = 0;
    int degreeTwoVertexCount = 0;
    int skeletonEdgeCount = 0;

    std::string message;
};

class KuratowskiSubdivisionVerifier {
public:
    KuratowskiSubdivisionVerification verify(
        const PreparedPalmTree& prepared,
        const std::vector<int>& originalEdgeIds
    ) const;

private:
    struct EdgeRecord {
        int u = -1;
        int v = -1;
        int originalEdgeId = -1;
        bool active = true;
    };

    static int maxOriginalEdgeId(const PreparedPalmTree& prepared);

    static std::vector<EdgeRecord> buildSelectedEdges(
        const PreparedPalmTree& prepared,
        const std::vector<int>& originalEdgeIds,
        std::vector<char>& selectedOriginalEdge
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

    static bool traceSkeleton(
        int vertexCount,
        const std::vector<EdgeRecord>& edges,
        const std::vector<std::vector<int>>& incidentEdges,
        const std::vector<int>& degree,
        const std::vector<char>& activeEdge,
        std::vector<int>& branchVertices,
        std::vector<std::pair<int, int>>& skeletonEdges,
        std::string& failureMessage
    );

    static bool classifyK5(
        int branchCount,
        const std::vector<std::pair<int, int>>& skeletonEdges,
        KuratowskiSubdivisionVerification& verification
    );

    static bool classifyK33(
        int branchCount,
        const std::vector<std::pair<int, int>>& skeletonEdges,
        KuratowskiSubdivisionVerification& verification
    );

    static bool isSkeletonBipartite(
        int branchCount,
        const std::vector<std::pair<int, int>>& skeletonEdges
    );

    static int otherEndpoint(
        const EdgeRecord& edge,
        int vertex
    );
};

} // namespace ht