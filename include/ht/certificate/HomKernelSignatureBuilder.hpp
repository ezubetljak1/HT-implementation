#pragma once

#include <string>
#include <vector>

#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

struct HomKernelSkeletonEdge {
    int id = -1;
    int u = -1;
    int v = -1;

    // Original graph edge IDs represented by this skeleton edge/path.
    std::vector<int> originalEdgeIds;
};

struct HomKernelSignature {
    bool valid = false;

    int branchVertexCount = 0;
    int skeletonEdgeCount = 0;

    // skeleton vertex id -> prepared/local vertex id
    std::vector<int> skeletonVertexToPreparedVertex;

    std::vector<HomKernelSkeletonEdge> skeletonEdges;

    // Stable shape key for this implementation.
    // Later this becomes the lookup key.
    std::string shapeKey;

    std::string message;
};

class HomKernelSignatureBuilder {
public:
    HomKernelSignature build(
        const PreparedPalmTree& prepared,
        const std::vector<int>& candidateOriginalEdgeIds
    ) const;

private:
    struct EdgeRecord {
        int u = -1;
        int v = -1;
        int originalEdgeId = -1;
    };

    static int maxOriginalEdgeId(
        const PreparedPalmTree& prepared
    );

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

    static bool traceSkeleton(
        int vertexCount,
        const std::vector<EdgeRecord>& edges,
        const std::vector<std::vector<int>>& incidentEdges,
        const std::vector<int>& degree,
        const std::vector<char>& activeEdge,
        HomKernelSignature& signature
    );

    static std::string buildShapeKey(
        const HomKernelSignature& signature
    );

    static int otherEndpoint(
        const EdgeRecord& edge,
        int vertex
    );
};

} // namespace ht