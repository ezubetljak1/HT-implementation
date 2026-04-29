#include "ht/certificate/HomKernelCanonicalizer.hpp"

#include <algorithm>
#include <sstream>

namespace ht {

CanonicalHomKernelSignature HomKernelCanonicalizer::canonicalize(
    const HomKernelSignature& signature
) const {
    CanonicalHomKernelSignature result;

    if (!signature.valid) {
        result.message = "Cannot canonicalize invalid HOMKERNEL signature.";
        return result;
    }

    const int vertexCount = signature.branchVertexCount;
    const int edgeCount = signature.skeletonEdgeCount;

    if (vertexCount <= 0 || edgeCount <= 0) {
        result.message = "Cannot canonicalize empty HOMKERNEL skeleton.";
        return result;
    }

    if (vertexCount > MaxCanonicalVertices || edgeCount > MaxCanonicalEdges) {
        result.message =
            "HOMKERNEL skeleton exceeds bounded canonicalization limits.";
        return result;
    }

    std::vector<EdgeView> edges =
        buildEdgeViews(signature);

    if (static_cast<int>(edges.size()) != edgeCount) {
        result.message =
            "HOMKERNEL skeleton edge list is inconsistent.";
        return result;
    }

    std::vector<int> degree =
        computeDegrees(vertexCount, edges);

    std::vector<int> candidateVertices =
        vertexOrderByDegree(degree);

    std::vector<char> used(
        static_cast<std::size_t>(vertexCount),
        0
    );

    std::vector<int> currentOrder;
    currentOrder.reserve(static_cast<std::size_t>(vertexCount));

    std::string bestKey;
    std::vector<int> bestOrder;
    std::vector<int> bestCanonicalEdgeToSkeletonEdge;

    backtrackCanonicalOrder(
        0,
        candidateVertices,
        degree,
        used,
        currentOrder,
        bestKey,
        bestOrder,
        bestCanonicalEdgeToSkeletonEdge,
        edges
    );

    if (bestKey.empty()) {
        result.message = "Canonical HOMKERNEL key could not be built.";
        return result;
    }

    result.valid = true;
    result.key = bestKey;
    result.canonicalVertexToSkeletonVertex = bestOrder;
    result.canonicalEdgeToSkeletonEdge = bestCanonicalEdgeToSkeletonEdge;
    result.message = "Canonical HOMKERNEL signature was built.";

    return result;
}

std::vector<HomKernelCanonicalizer::EdgeView>
HomKernelCanonicalizer::buildEdgeViews(
    const HomKernelSignature& signature
) {
    std::vector<EdgeView> edges;

    for (const HomKernelSkeletonEdge& skeletonEdge : signature.skeletonEdges) {
        EdgeView edge;
        edge.u = skeletonEdge.u;
        edge.v = skeletonEdge.v;
        edge.skeletonEdgeId = skeletonEdge.id;

        edges.push_back(edge);
    }

    return edges;
}

std::vector<int> HomKernelCanonicalizer::computeDegrees(
    int vertexCount,
    const std::vector<EdgeView>& edges
) {
    std::vector<int> degree(
        static_cast<std::size_t>(vertexCount),
        0
    );

    for (const EdgeView& edge : edges) {
        if (edge.u >= 0 && edge.u < vertexCount) {
            ++degree[edge.u];
        }

        if (edge.v >= 0 && edge.v < vertexCount) {
            ++degree[edge.v];
        }
    }

    return degree;
}

std::vector<int> HomKernelCanonicalizer::vertexOrderByDegree(
    const std::vector<int>& degree
) {
    std::vector<int> vertices(
        degree.size(),
        0
    );

    for (int i = 0; i < static_cast<int>(vertices.size()); ++i) {
        vertices[i] = i;
    }

    std::sort(
        vertices.begin(),
        vertices.end(),
        [&degree](int left, int right) {
            if (degree[left] != degree[right]) {
                return degree[left] > degree[right];
            }

            return left < right;
        }
    );

    return vertices;
}

void HomKernelCanonicalizer::backtrackCanonicalOrder(
    int position,
    const std::vector<int>& candidateVertices,
    const std::vector<int>& degree,
    std::vector<char>& used,
    std::vector<int>& currentOrder,
    std::string& bestKey,
    std::vector<int>& bestOrder,
    std::vector<int>& bestCanonicalEdgeToSkeletonEdge,
    const std::vector<EdgeView>& edges
) {
    const int vertexCount =
        static_cast<int>(candidateVertices.size());

    if (position == vertexCount) {
        std::vector<int> canonicalEdgeToSkeletonEdge;

        std::string key =
            buildKeyForOrder(
                vertexCount,
                currentOrder,
                edges,
                canonicalEdgeToSkeletonEdge
            );

        if (bestKey.empty() || key < bestKey) {
            bestKey = key;
            bestOrder = currentOrder;
            bestCanonicalEdgeToSkeletonEdge =
                canonicalEdgeToSkeletonEdge;
        }

        return;
    }

    for (int vertex : candidateVertices) {
        if (vertex < 0 || vertex >= vertexCount) {
            continue;
        }

        if (used[vertex]) {
            continue;
        }

        if (!degreeCompatiblePrefix(
                currentOrder,
                vertex,
                degree
            )) {
            continue;
        }

        used[vertex] = 1;
        currentOrder.push_back(vertex);

        backtrackCanonicalOrder(
            position + 1,
            candidateVertices,
            degree,
            used,
            currentOrder,
            bestKey,
            bestOrder,
            bestCanonicalEdgeToSkeletonEdge,
            edges
        );

        currentOrder.pop_back();
        used[vertex] = 0;
    }
}

bool HomKernelCanonicalizer::degreeCompatiblePrefix(
    const std::vector<int>& currentOrder,
    int candidateVertex,
    const std::vector<int>& degree
) {
    if (currentOrder.empty()) {
        return true;
    }

    const int previousVertex =
        currentOrder.back();

    // Canonical order groups vertices by non-increasing degree.
    // This prunes many equivalent permutations while still allowing
    // all same-degree vertices to permute.
    if (degree[candidateVertex] > degree[previousVertex]) {
        return false;
    }

    return true;
}

std::string HomKernelCanonicalizer::buildKeyForOrder(
    int vertexCount,
    const std::vector<int>& order,
    const std::vector<EdgeView>& edges,
    std::vector<int>& canonicalEdgeToSkeletonEdge
) {
    std::vector<int> oldToCanonical(
        static_cast<std::size_t>(vertexCount),
        -1
    );

    for (int canonicalVertex = 0;
         canonicalVertex < static_cast<int>(order.size());
         ++canonicalVertex) {
        const int oldVertex = order[canonicalVertex];

        if (oldVertex >= 0 && oldVertex < vertexCount) {
            oldToCanonical[oldVertex] = canonicalVertex;
        }
    }

    struct CanonicalEdge {
        int u = -1;
        int v = -1;
        int skeletonEdgeId = -1;
    };

    std::vector<CanonicalEdge> canonicalEdges;

    for (const EdgeView& edge : edges) {
        if (edge.u < 0 || edge.u >= vertexCount
            || edge.v < 0 || edge.v >= vertexCount) {
            continue;
        }

        int u = oldToCanonical[edge.u];
        int v = oldToCanonical[edge.v];

        if (u > v) {
            std::swap(u, v);
        }

        CanonicalEdge canonicalEdge;
        canonicalEdge.u = u;
        canonicalEdge.v = v;
        canonicalEdge.skeletonEdgeId = edge.skeletonEdgeId;

        canonicalEdges.push_back(canonicalEdge);
    }

    std::sort(
        canonicalEdges.begin(),
        canonicalEdges.end(),
        [](const CanonicalEdge& left, const CanonicalEdge& right) {
            if (left.u != right.u) {
                return left.u < right.u;
            }

            if (left.v != right.v) {
                return left.v < right.v;
            }

            return left.skeletonEdgeId < right.skeletonEdgeId;
        }
    );

    canonicalEdgeToSkeletonEdge.clear();

    std::ostringstream out;

    out << "V=" << vertexCount
        << ";E=" << canonicalEdges.size()
        << ";edges=";

    for (const CanonicalEdge& edge : canonicalEdges) {
        out << edge.u << "-" << edge.v << ",";
        canonicalEdgeToSkeletonEdge.push_back(edge.skeletonEdgeId);
    }

    return out.str();
}

int HomKernelCanonicalizer::indexOfVertexInOrder(
    const std::vector<int>& order,
    int vertex
) {
    for (int i = 0; i < static_cast<int>(order.size()); ++i) {
        if (order[i] == vertex) {
            return i;
        }
    }

    return -1;
}

} // namespace ht