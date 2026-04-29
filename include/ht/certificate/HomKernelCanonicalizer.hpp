#pragma once

#include <string>
#include <vector>

#include "ht/certificate/HomKernelSignatureBuilder.hpp"

namespace ht {

struct CanonicalHomKernelSignature {
    bool valid = false;

    std::string key;

    // canonical edge index -> HomKernelSignature::skeletonEdges[index].id
    std::vector<int> canonicalEdgeToSkeletonEdge;

    // canonical vertex index -> original skeleton vertex index
    std::vector<int> canonicalVertexToSkeletonVertex;

    std::string message;
};

class HomKernelCanonicalizer {
public:
    CanonicalHomKernelSignature canonicalize(
        const HomKernelSignature& signature
    ) const;

private:
    struct EdgeView {
        int u = -1;
        int v = -1;
        int skeletonEdgeId = -1;
    };

    static constexpr int MaxCanonicalVertices = 18;
    static constexpr int MaxCanonicalEdges = 27;

    static std::vector<EdgeView> buildEdgeViews(
        const HomKernelSignature& signature
    );

    static std::vector<int> computeDegrees(
        int vertexCount,
        const std::vector<EdgeView>& edges
    );

    static std::vector<int> vertexOrderByDegree(
        const std::vector<int>& degree
    );

    static void backtrackCanonicalOrder(
        int position,
        const std::vector<int>& candidateVertices,
        const std::vector<int>& degree,
        std::vector<char>& used,
        std::vector<int>& currentOrder,
        std::string& bestKey,
        std::vector<int>& bestOrder,
        std::vector<int>& bestCanonicalEdgeToSkeletonEdge,
        const std::vector<EdgeView>& edges
    );

    static bool degreeCompatiblePrefix(
        const std::vector<int>& currentOrder,
        int candidateVertex,
        const std::vector<int>& degree
    );

    static std::string buildKeyForOrder(
        int vertexCount,
        const std::vector<int>& order,
        const std::vector<EdgeView>& edges,
        std::vector<int>& canonicalEdgeToSkeletonEdge
    );

    static int indexOfVertexInOrder(
        const std::vector<int>& order,
        int vertex
    );
};

} // namespace ht