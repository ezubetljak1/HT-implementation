#include "ht/certificate/KuratowskiKernelSelector.hpp"

#include <queue>

namespace ht {

KuratowskiSubdivisionVerification KuratowskiKernelSelector::select(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds
) const {
    KuratowskiSubdivisionVerification result;

    KuratowskiSubdivisionVerifier verifier;
    KuratowskiSubdivisionVerification direct =
        verifier.verify(
            prepared,
            candidateOriginalEdgeIds
        );

    if (direct.valid) {
        return direct;
    }

    std::vector<EdgeRecord> edges =
        buildSelectedEdges(
            prepared,
            candidateOriginalEdgeIds
        );

    if (edges.empty()) {
        result.message = "Kernel selector received an empty candidate edge set.";
        return result;
    }

    std::vector<int> degree;
    std::vector<std::vector<int>> incidentEdges =
        buildIncidentEdges(
            prepared.n,
            edges,
            degree
        );

    std::vector<char> activeEdge(edges.size(), 1);

    pruneLeaves(
        edges,
        incidentEdges,
        degree,
        activeEdge
    );

    std::vector<SkeletonEdge> skeletonEdges;
    std::string skeletonMessage;

    if (!buildSkeleton(
            prepared.n,
            edges,
            incidentEdges,
            degree,
            activeEdge,
            skeletonEdges,
            skeletonMessage
        )) {
        result.message = skeletonMessage;
        return result;
    }

    if (skeletonEdges.empty()) {
        result.message = "Kernel selector produced an empty skeleton.";
        return result;
    }

    if (static_cast<int>(skeletonEdges.size()) > MaxSkeletonEdgesForBoundedSelection) {
        result.message =
            "Reduced kernel skeleton is too large for bounded Williamson selector.";
        return result;
    }

    for (int subsetSize = 9;
         subsetSize <= static_cast<int>(skeletonEdges.size());
         ++subsetSize) {
        KuratowskiSubdivisionVerification selected;

        if (trySubsetsOfSize(
                prepared,
                skeletonEdges,
                subsetSize,
                selected
            )) {
            return selected;
        }
    }

    result.message =
        "No K5 or K3,3 subdivision was found inside the bounded kernel skeleton.";
    return result;
}

int KuratowskiKernelSelector::maxOriginalEdgeId(
    const PreparedPalmTree& prepared
) {
    int maxId = -1;

    for (const Dart& dart : prepared.darts) {
        if (dart.originalEdgeId > maxId) {
            maxId = dart.originalEdgeId;
        }
    }

    return maxId;
}

std::vector<KuratowskiKernelSelector::EdgeRecord>
KuratowskiKernelSelector::buildSelectedEdges(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds
) {
    const int maxId = maxOriginalEdgeId(prepared);

    if (maxId < 0) {
        return {};
    }

    std::vector<char> selectedOriginalEdge(
        static_cast<std::size_t>(maxId + 1),
        0
    );

    for (int originalEdgeId : candidateOriginalEdgeIds) {
        if (originalEdgeId < 0 || originalEdgeId > maxId) {
            continue;
        }

        selectedOriginalEdge[originalEdgeId] = 1;
    }

    std::vector<char> added(
        static_cast<std::size_t>(maxId + 1),
        0
    );

    std::vector<EdgeRecord> edges;

    for (const Dart& dart : prepared.darts) {
        const int originalEdgeId = dart.originalEdgeId;

        if (originalEdgeId < 0 || originalEdgeId > maxId) {
            continue;
        }

        if (!selectedOriginalEdge[originalEdgeId]) {
            continue;
        }

        if (added[originalEdgeId]) {
            continue;
        }

        EdgeRecord edge;
        edge.u = dart.from;
        edge.v = dart.to;
        edge.originalEdgeId = originalEdgeId;

        edges.push_back(edge);
        added[originalEdgeId] = 1;
    }

    return edges;
}

std::vector<std::vector<int>>
KuratowskiKernelSelector::buildIncidentEdges(
    int vertexCount,
    const std::vector<EdgeRecord>& edges,
    std::vector<int>& degree
) {
    std::vector<std::vector<int>> incidentEdges(
        static_cast<std::size_t>(vertexCount)
    );

    degree.assign(vertexCount, 0);

    for (int edgeIndex = 0;
         edgeIndex < static_cast<int>(edges.size());
         ++edgeIndex) {
        const EdgeRecord& edge = edges[edgeIndex];

        if (edge.u < 0 || edge.u >= vertexCount
            || edge.v < 0 || edge.v >= vertexCount) {
            continue;
        }

        incidentEdges[edge.u].push_back(edgeIndex);
        incidentEdges[edge.v].push_back(edgeIndex);

        ++degree[edge.u];
        ++degree[edge.v];
    }

    return incidentEdges;
}

void KuratowskiKernelSelector::pruneLeaves(
    const std::vector<EdgeRecord>& edges,
    const std::vector<std::vector<int>>& incidentEdges,
    std::vector<int>& degree,
    std::vector<char>& activeEdge
) {
    std::queue<int> queue;

    for (int vertex = 0;
         vertex < static_cast<int>(degree.size());
         ++vertex) {
        if (degree[vertex] == 1) {
            queue.push(vertex);
        }
    }

    while (!queue.empty()) {
        int vertex = queue.front();
        queue.pop();

        if (vertex < 0 || vertex >= static_cast<int>(degree.size())) {
            continue;
        }

        if (degree[vertex] != 1) {
            continue;
        }

        for (int edgeIndex : incidentEdges[vertex]) {
            if (edgeIndex < 0 || edgeIndex >= static_cast<int>(edges.size())) {
                continue;
            }

            if (!activeEdge[edgeIndex]) {
                continue;
            }

            activeEdge[edgeIndex] = 0;

            int other =
                otherEndpoint(
                    edges[edgeIndex],
                    vertex
                );

            if (degree[vertex] > 0) {
                --degree[vertex];
            }

            if (other >= 0
                && other < static_cast<int>(degree.size())
                && degree[other] > 0) {
                --degree[other];

                if (degree[other] == 1) {
                    queue.push(other);
                }
            }

            break;
        }
    }
}

bool KuratowskiKernelSelector::buildSkeleton(
    int vertexCount,
    const std::vector<EdgeRecord>& edges,
    const std::vector<std::vector<int>>& incidentEdges,
    const std::vector<int>& degree,
    const std::vector<char>& activeEdge,
    std::vector<SkeletonEdge>& skeletonEdges,
    std::string& message
) {
    std::vector<int> branchIndex(
        static_cast<std::size_t>(vertexCount),
        -1
    );

    int branchCount = 0;

    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        if (degree[vertex] <= 0) {
            continue;
        }

        if (degree[vertex] != 2) {
            branchIndex[vertex] = branchCount++;
        }
    }

    if (branchCount == 0) {
        message = "Kernel skeleton has no branch vertices.";
        return false;
    }

    std::vector<char> visitedEdge(edges.size(), 0);

    for (int startVertex = 0; startVertex < vertexCount; ++startVertex) {
        if (branchIndex[startVertex] == -1) {
            continue;
        }

        for (int startEdge : incidentEdges[startVertex]) {
            if (startEdge < 0 || startEdge >= static_cast<int>(edges.size())) {
                continue;
            }

            if (!activeEdge[startEdge] || visitedEdge[startEdge]) {
                continue;
            }

            SkeletonEdge skeletonEdge;
            skeletonEdge.u = branchIndex[startVertex];

            int previousVertex = startVertex;
            int currentEdge = startEdge;

            while (true) {
                if (currentEdge < 0
                    || currentEdge >= static_cast<int>(edges.size())) {
                    message = "Invalid edge while tracing kernel skeleton.";
                    return false;
                }

                if (!activeEdge[currentEdge]) {
                    message = "Inactive edge reached while tracing kernel skeleton.";
                    return false;
                }

                visitedEdge[currentEdge] = 1;
                skeletonEdge.originalEdgeIds.push_back(
                    edges[currentEdge].originalEdgeId
                );

                const int nextVertex =
                    otherEndpoint(
                        edges[currentEdge],
                        previousVertex
                    );

                if (nextVertex < 0 || nextVertex >= vertexCount) {
                    message = "Invalid vertex while tracing kernel skeleton.";
                    return false;
                }

                if (branchIndex[nextVertex] != -1) {
                    skeletonEdge.v = branchIndex[nextVertex];
                    skeletonEdges.push_back(skeletonEdge);
                    break;
                }

                if (degree[nextVertex] != 2) {
                    message =
                        "Internal skeleton vertex is not degree 2.";
                    return false;
                }

                int nextEdge = -1;

                for (int incidentEdge : incidentEdges[nextVertex]) {
                    if (!activeEdge[incidentEdge]) {
                        continue;
                    }

                    if (incidentEdge == currentEdge) {
                        continue;
                    }

                    nextEdge = incidentEdge;
                    break;
                }

                if (nextEdge == -1) {
                    message =
                        "Could not continue degree-2 chain in kernel skeleton.";
                    return false;
                }

                previousVertex = nextVertex;
                currentEdge = nextEdge;
            }
        }
    }

    return true;
}

bool KuratowskiKernelSelector::trySubsetsOfSize(
    const PreparedPalmTree& prepared,
    const std::vector<SkeletonEdge>& skeletonEdges,
    int targetSize,
    KuratowskiSubdivisionVerification& selected
) {
    std::vector<int> chosen;
    chosen.reserve(static_cast<std::size_t>(targetSize));

    return chooseSubsetRecursive(
        prepared,
        skeletonEdges,
        targetSize,
        0,
        chosen,
        selected
    );
}

bool KuratowskiKernelSelector::chooseSubsetRecursive(
    const PreparedPalmTree& prepared,
    const std::vector<SkeletonEdge>& skeletonEdges,
    int targetSize,
    int index,
    std::vector<int>& chosen,
    KuratowskiSubdivisionVerification& selected
) {
    if (static_cast<int>(chosen.size()) == targetSize) {
        std::vector<int> originalEdgeIds =
            expandSkeletonSubset(
                skeletonEdges,
                chosen
            );

        KuratowskiSubdivisionVerifier verifier;
        selected =
            verifier.verify(
                prepared,
                originalEdgeIds
            );

        return selected.valid;
    }

    if (index >= static_cast<int>(skeletonEdges.size())) {
        return false;
    }

    const int remaining =
        static_cast<int>(skeletonEdges.size()) - index;

    const int needed =
        targetSize - static_cast<int>(chosen.size());

    if (remaining < needed) {
        return false;
    }

    chosen.push_back(index);

    if (chooseSubsetRecursive(
            prepared,
            skeletonEdges,
            targetSize,
            index + 1,
            chosen,
            selected
        )) {
        return true;
    }

    chosen.pop_back();

    return chooseSubsetRecursive(
        prepared,
        skeletonEdges,
        targetSize,
        index + 1,
        chosen,
        selected
    );
}

std::vector<int> KuratowskiKernelSelector::expandSkeletonSubset(
    const std::vector<SkeletonEdge>& skeletonEdges,
    const std::vector<int>& chosen
) {
    std::vector<int> originalEdgeIds;

    for (int skeletonEdgeIndex : chosen) {
        if (skeletonEdgeIndex < 0
            || skeletonEdgeIndex >= static_cast<int>(skeletonEdges.size())) {
            continue;
        }

        const SkeletonEdge& skeletonEdge =
            skeletonEdges[skeletonEdgeIndex];

        originalEdgeIds.insert(
            originalEdgeIds.end(),
            skeletonEdge.originalEdgeIds.begin(),
            skeletonEdge.originalEdgeIds.end()
        );
    }

    return originalEdgeIds;
}

int KuratowskiKernelSelector::otherEndpoint(
    const EdgeRecord& edge,
    int vertex
) {
    if (edge.u == vertex) {
        return edge.v;
    }

    if (edge.v == vertex) {
        return edge.u;
    }

    return -1;
}

} // namespace ht