#include "ht/certificate/HomKernelSignatureBuilder.hpp"

#include <queue>
#include <sstream>

namespace ht {

HomKernelSignature HomKernelSignatureBuilder::build(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds
) const {
    HomKernelSignature signature;

    std::vector<EdgeRecord> edges =
        buildSelectedEdges(
            prepared,
            candidateOriginalEdgeIds
        );

    if (edges.empty()) {
        signature.message =
            "Cannot build HOMKERNEL signature from empty candidate edge set.";
        return signature;
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

    if (!traceSkeleton(
            prepared.n,
            edges,
            incidentEdges,
            degree,
            activeEdge,
            signature
        )) {
        return signature;
    }

    signature.shapeKey =
        buildShapeKey(signature);

    signature.valid = true;
    signature.message = "HOMKERNEL skeleton signature was built.";

    return signature;
}

int HomKernelSignatureBuilder::maxOriginalEdgeId(
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

std::vector<HomKernelSignatureBuilder::EdgeRecord>
HomKernelSignatureBuilder::buildSelectedEdges(
    const PreparedPalmTree& prepared,
    const std::vector<int>& candidateOriginalEdgeIds
) {
    const int maxId = maxOriginalEdgeId(prepared);

    if (maxId < 0) {
        return {};
    }

    std::vector<char> selected(
        static_cast<std::size_t>(maxId + 1),
        0
    );

    for (int originalEdgeId : candidateOriginalEdgeIds) {
        if (originalEdgeId < 0 || originalEdgeId > maxId) {
            continue;
        }

        selected[originalEdgeId] = 1;
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

        if (!selected[originalEdgeId]) {
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
HomKernelSignatureBuilder::buildIncidentEdges(
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

void HomKernelSignatureBuilder::pruneLeaves(
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
        const int vertex = queue.front();
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

            const int other =
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

bool HomKernelSignatureBuilder::traceSkeleton(
    int vertexCount,
    const std::vector<EdgeRecord>& edges,
    const std::vector<std::vector<int>>& incidentEdges,
    const std::vector<int>& degree,
    const std::vector<char>& activeEdge,
    HomKernelSignature& signature
) {
    std::vector<int> branchIndex(
        static_cast<std::size_t>(vertexCount),
        -1
    );

    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        if (degree[vertex] <= 0) {
            continue;
        }

        if (degree[vertex] != 2) {
            branchIndex[vertex] =
                static_cast<int>(
                    signature.skeletonVertexToPreparedVertex.size()
                );

            signature.skeletonVertexToPreparedVertex.push_back(vertex);
        }
    }

    if (signature.skeletonVertexToPreparedVertex.empty()) {
        signature.message = "HOMKERNEL skeleton has no branch vertices.";
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

            HomKernelSkeletonEdge skeletonEdge;
            skeletonEdge.id =
                static_cast<int>(signature.skeletonEdges.size());
            skeletonEdge.u = branchIndex[startVertex];

            int previousVertex = startVertex;
            int currentEdge = startEdge;

            while (true) {
                if (currentEdge < 0
                    || currentEdge >= static_cast<int>(edges.size())) {
                    signature.message =
                        "Invalid edge while tracing HOMKERNEL skeleton.";
                    return false;
                }

                if (!activeEdge[currentEdge]) {
                    signature.message =
                        "Inactive edge reached while tracing HOMKERNEL skeleton.";
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
                    signature.message =
                        "Invalid vertex while tracing HOMKERNEL skeleton.";
                    return false;
                }

                if (branchIndex[nextVertex] != -1) {
                    skeletonEdge.v = branchIndex[nextVertex];
                    signature.skeletonEdges.push_back(skeletonEdge);
                    break;
                }

                if (degree[nextVertex] != 2) {
                    signature.message =
                        "Internal HOMKERNEL skeleton vertex is not degree 2.";
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
                    signature.message =
                        "Could not continue degree-2 chain in HOMKERNEL skeleton.";
                    return false;
                }

                previousVertex = nextVertex;
                currentEdge = nextEdge;
            }
        }
    }

    signature.branchVertexCount =
        static_cast<int>(
            signature.skeletonVertexToPreparedVertex.size()
        );

    signature.skeletonEdgeCount =
        static_cast<int>(signature.skeletonEdges.size());

    return true;
}

std::string HomKernelSignatureBuilder::buildShapeKey(
    const HomKernelSignature& signature
) {
    std::vector<int> degree(
        static_cast<std::size_t>(signature.branchVertexCount),
        0
    );

    for (const HomKernelSkeletonEdge& edge : signature.skeletonEdges) {
        if (edge.u >= 0 && edge.u < signature.branchVertexCount) {
            ++degree[edge.u];
        }

        if (edge.v >= 0 && edge.v < signature.branchVertexCount) {
            ++degree[edge.v];
        }
    }

    std::ostringstream out;

    out << "V=" << signature.branchVertexCount
        << ";E=" << signature.skeletonEdgeCount
        << ";deg=";

    for (int value : degree) {
        out << value << ",";
    }

    out << ";edges=";

    for (const HomKernelSkeletonEdge& edge : signature.skeletonEdges) {
        out << edge.u << "-" << edge.v
            << "[" << edge.originalEdgeIds.size() << "]"
            << ",";
    }

    return out.str();
}

int HomKernelSignatureBuilder::otherEndpoint(
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