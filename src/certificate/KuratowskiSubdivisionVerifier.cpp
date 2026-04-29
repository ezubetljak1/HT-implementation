#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"

#include <queue>
#include <stdexcept>

namespace ht {

KuratowskiSubdivisionVerification KuratowskiSubdivisionVerifier::verify(
    const PreparedPalmTree& prepared,
    const std::vector<int>& originalEdgeIds
) const {
    KuratowskiSubdivisionVerification verification;

    if (originalEdgeIds.empty()) {
        verification.message = "Cannot verify empty Kuratowski edge set.";
        return verification;
    }

    const int maxOriginalId = maxOriginalEdgeId(prepared);

    if (maxOriginalId < 0) {
        verification.message = "Prepared graph has no original edge IDs.";
        return verification;
    }

    std::vector<char> selectedOriginalEdge(
        static_cast<std::size_t>(maxOriginalId + 1),
        0
    );

    std::vector<EdgeRecord> edges =
        buildSelectedEdges(
            prepared,
            originalEdgeIds,
            selectedOriginalEdge
        );

    if (edges.empty()) {
        verification.message =
            "Selected original edge IDs do not correspond to prepared darts.";
        return verification;
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

    std::vector<int> branchVertices;
    std::vector<std::pair<int, int>> skeletonEdges;
    std::string failureMessage;

    if (!traceSkeleton(
            prepared.n,
            edges,
            incidentEdges,
            degree,
            activeEdge,
            branchVertices,
            skeletonEdges,
            failureMessage
        )) {
        verification.message = failureMessage;
        return verification;
    }

    verification.branchVertexCount =
        static_cast<int>(branchVertices.size());

    verification.skeletonEdgeCount =
        static_cast<int>(skeletonEdges.size());

    for (int v = 0; v < prepared.n; ++v) {
        if (degree[v] <= 0) {
            continue;
        }

        ++verification.activeVertexCount;

        if (degree[v] == 2) {
            ++verification.degreeTwoVertexCount;
        }
    }

    for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
        if (!activeEdge[i]) {
            continue;
        }

        verification.originalEdgeIds.push_back(edges[i].originalEdgeId);
        ++verification.activeEdgeCount;
    }

    if (classifyK5(
            verification.branchVertexCount,
            skeletonEdges,
            verification
        )) {
        verification.valid = true;
        verification.type = KuratowskiType::K5Subdivision;
        verification.message = "Verified Kuratowski subdivision of K5.";
        return verification;
    }

    if (classifyK33(
            verification.branchVertexCount,
            skeletonEdges,
            verification
        )) {
        verification.valid = true;
        verification.type = KuratowskiType::K33Subdivision;
        verification.message = "Verified Kuratowski subdivision of K3,3.";
        return verification;
    }

    verification.message =
        "Candidate is not recognized as a K5 or K3,3 subdivision after suppressing degree-2 paths.";

    return verification;
}

int KuratowskiSubdivisionVerifier::maxOriginalEdgeId(
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

std::vector<KuratowskiSubdivisionVerifier::EdgeRecord>
KuratowskiSubdivisionVerifier::buildSelectedEdges(
    const PreparedPalmTree& prepared,
    const std::vector<int>& originalEdgeIds,
    std::vector<char>& selectedOriginalEdge
) {
    for (int originalEdgeId : originalEdgeIds) {
        if (originalEdgeId < 0
            || originalEdgeId >= static_cast<int>(selectedOriginalEdge.size())) {
            continue;
        }

        selectedOriginalEdge[originalEdgeId] = 1;
    }

    std::vector<char> alreadyAdded(selectedOriginalEdge.size(), 0);
    std::vector<EdgeRecord> edges;

    for (const Dart& dart : prepared.darts) {
        const int originalEdgeId = dart.originalEdgeId;

        if (originalEdgeId < 0
            || originalEdgeId >= static_cast<int>(selectedOriginalEdge.size())) {
            continue;
        }

        if (!selectedOriginalEdge[originalEdgeId]) {
            continue;
        }

        if (alreadyAdded[originalEdgeId]) {
            continue;
        }

        EdgeRecord edge;
        edge.u = dart.from;
        edge.v = dart.to;
        edge.originalEdgeId = originalEdgeId;
        edge.active = true;

        edges.push_back(edge);
        alreadyAdded[originalEdgeId] = 1;
    }

    return edges;
}

std::vector<std::vector<int>>
KuratowskiSubdivisionVerifier::buildIncidentEdges(
    int vertexCount,
    const std::vector<EdgeRecord>& edges,
    std::vector<int>& degree
) {
    std::vector<std::vector<int>> incidentEdges(
        static_cast<std::size_t>(vertexCount)
    );

    degree.assign(vertexCount, 0);

    for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
        const EdgeRecord& edge = edges[i];

        if (edge.u < 0 || edge.u >= vertexCount
            || edge.v < 0 || edge.v >= vertexCount) {
            throw std::runtime_error("Invalid edge endpoint in KuratowskiSubdivisionVerifier.");
        }

        incidentEdges[edge.u].push_back(i);
        incidentEdges[edge.v].push_back(i);

        ++degree[edge.u];
        ++degree[edge.v];
    }

    return incidentEdges;
}

void KuratowskiSubdivisionVerifier::pruneLeaves(
    const std::vector<EdgeRecord>& edges,
    const std::vector<std::vector<int>>& incidentEdges,
    std::vector<int>& degree,
    std::vector<char>& activeEdge
) {
    std::queue<int> queue;

    for (int v = 0; v < static_cast<int>(degree.size()); ++v) {
        if (degree[v] == 1) {
            queue.push(v);
        }
    }

    while (!queue.empty()) {
        const int v = queue.front();
        queue.pop();

        if (v < 0 || v >= static_cast<int>(degree.size())) {
            continue;
        }

        if (degree[v] != 1) {
            continue;
        }

        for (int edgeIndex : incidentEdges[v]) {
            if (edgeIndex < 0 || edgeIndex >= static_cast<int>(edges.size())) {
                continue;
            }

            if (!activeEdge[edgeIndex]) {
                continue;
            }

            activeEdge[edgeIndex] = 0;

            const int other = otherEndpoint(edges[edgeIndex], v);

            if (degree[v] > 0) {
                --degree[v];
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

bool KuratowskiSubdivisionVerifier::traceSkeleton(
    int vertexCount,
    const std::vector<EdgeRecord>& edges,
    const std::vector<std::vector<int>>& incidentEdges,
    const std::vector<int>& degree,
    const std::vector<char>& activeEdge,
    std::vector<int>& branchVertices,
    std::vector<std::pair<int, int>>& skeletonEdges,
    std::string& failureMessage
) {
    std::vector<int> branchIndex(vertexCount, -1);

    for (int v = 0; v < vertexCount; ++v) {
        if (degree[v] <= 0) {
            continue;
        }

        if (degree[v] == 1) {
            failureMessage = "Leaf remained after pruning.";
            return false;
        }

        if (degree[v] != 2) {
            branchIndex[v] = static_cast<int>(branchVertices.size());
            branchVertices.push_back(v);
        }
    }

    if (branchVertices.size() != 5 && branchVertices.size() != 6) {
        failureMessage =
            "Suppressed skeleton does not have 5 or 6 branch vertices.";
        return false;
    }

    std::vector<char> visitedEdge(edges.size(), 0);

    for (int branchVertex : branchVertices) {
        for (int startEdge : incidentEdges[branchVertex]) {
            if (startEdge < 0 || startEdge >= static_cast<int>(edges.size())) {
                continue;
            }

            if (!activeEdge[startEdge] || visitedEdge[startEdge]) {
                continue;
            }

            int previousVertex = branchVertex;
            int currentEdge = startEdge;

            while (true) {
                if (currentEdge < 0
                    || currentEdge >= static_cast<int>(edges.size())) {
                    failureMessage = "Invalid edge while tracing skeleton.";
                    return false;
                }

                if (!activeEdge[currentEdge]) {
                    failureMessage = "Inactive edge reached while tracing skeleton.";
                    return false;
                }

                visitedEdge[currentEdge] = 1;

                const int nextVertex =
                    otherEndpoint(edges[currentEdge], previousVertex);

                if (nextVertex < 0 || nextVertex >= vertexCount) {
                    failureMessage = "Invalid vertex while tracing skeleton.";
                    return false;
                }

                if (branchIndex[nextVertex] != -1) {
                    skeletonEdges.push_back(
                        {
                            branchIndex[branchVertex],
                            branchIndex[nextVertex]
                        }
                    );
                    break;
                }

                if (degree[nextVertex] != 2) {
                    failureMessage =
                        "Non-branch internal vertex does not have degree 2.";
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
                    failureMessage =
                        "Could not continue degree-2 chain while tracing skeleton.";
                    return false;
                }

                previousVertex = nextVertex;
                currentEdge = nextEdge;
            }
        }
    }

    return true;
}

bool KuratowskiSubdivisionVerifier::classifyK5(
    int branchCount,
    const std::vector<std::pair<int, int>>& skeletonEdges,
    KuratowskiSubdivisionVerification& /* verification */
) {
    if (branchCount != 5) {
        return false;
    }

    if (skeletonEdges.size() != 10) {
        return false;
    }

    std::vector<int> degree(5, 0);
    std::vector<std::vector<char>> seen(5, std::vector<char>(5, 0));

    for (const auto& edge : skeletonEdges) {
        int u = edge.first;
        int v = edge.second;

        if (u < 0 || u >= 5 || v < 0 || v >= 5 || u == v) {
            return false;
        }

        if (seen[u][v] || seen[v][u]) {
            return false;
        }

        seen[u][v] = 1;
        seen[v][u] = 1;

        ++degree[u];
        ++degree[v];
    }

    for (int d : degree) {
        if (d != 4) {
            return false;
        }
    }

    return true;
}

bool KuratowskiSubdivisionVerifier::classifyK33(
    int branchCount,
    const std::vector<std::pair<int, int>>& skeletonEdges,
    KuratowskiSubdivisionVerification& /* verification */
) {
    if (branchCount != 6) {
        return false;
    }

    if (skeletonEdges.size() != 9) {
        return false;
    }

    std::vector<int> degree(6, 0);
    std::vector<std::vector<char>> seen(6, std::vector<char>(6, 0));

    for (const auto& edge : skeletonEdges) {
        int u = edge.first;
        int v = edge.second;

        if (u < 0 || u >= 6 || v < 0 || v >= 6 || u == v) {
            return false;
        }

        if (seen[u][v] || seen[v][u]) {
            return false;
        }

        seen[u][v] = 1;
        seen[v][u] = 1;

        ++degree[u];
        ++degree[v];
    }

    for (int d : degree) {
        if (d != 3) {
            return false;
        }
    }

    return isSkeletonBipartite(
        branchCount,
        skeletonEdges
    );
}

bool KuratowskiSubdivisionVerifier::isSkeletonBipartite(
    int branchCount,
    const std::vector<std::pair<int, int>>& skeletonEdges
) {
    std::vector<std::vector<int>> adjacency(branchCount);

    for (const auto& edge : skeletonEdges) {
        adjacency[edge.first].push_back(edge.second);
        adjacency[edge.second].push_back(edge.first);
    }

    std::vector<int> color(branchCount, -1);
    std::queue<int> queue;

    for (int start = 0; start < branchCount; ++start) {
        if (color[start] != -1) {
            continue;
        }

        color[start] = 0;
        queue.push(start);

        while (!queue.empty()) {
            int v = queue.front();
            queue.pop();

            for (int next : adjacency[v]) {
                if (color[next] == -1) {
                    color[next] = 1 - color[v];
                    queue.push(next);
                    continue;
                }

                if (color[next] == color[v]) {
                    return false;
                }
            }
        }
    }

    return true;
}

int KuratowskiSubdivisionVerifier::otherEndpoint(
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