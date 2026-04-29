#include "ht/embedding/EmbeddingValidator.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

namespace ht {

EmbeddingValidationResult EmbeddingValidator::validate(
    const PreparedPalmTree& prepared,
    const Embedding& embedding
) {
    EmbeddingValidationResult result;

    result.vertexCount = prepared.n;
    result.edgeCount = static_cast<int>(prepared.darts.size()) / 2;
    result.expectedFaceCount = result.edgeCount - result.vertexCount + 2;

    if (prepared.n <= 0) {
        return fail("Prepared graph has no vertices.", result);
    }

    if (prepared.darts.empty()) {
        return fail("Prepared graph has no darts.", result);
    }

    if (prepared.darts.size() % 2 != 0) {
        return fail("Prepared graph has odd number of darts.", result);
    }

    if (static_cast<int>(embedding.rotationDarts.size()) != prepared.n) {
        return fail("Rotation system has wrong number of vertices.", result);
    }

    std::vector<int> positionOfDart(prepared.darts.size(), -1);

    for (int v = 0; v < prepared.n; ++v) {
        const auto& rotation = embedding.rotationDarts[v];

        if (rotation.size() != prepared.outAll[v].size()) {
            std::ostringstream oss;
            oss << "Vertex " << v
                << " has rotation size " << rotation.size()
                << ", but expected degree " << prepared.outAll[v].size()
                << ".";

            return fail(oss.str(), result);
        }

        std::vector<bool> expectedAtVertex(prepared.darts.size(), false);

        for (int d : prepared.outAll[v]) {
            expectedAtVertex[d] = true;
        }

        for (int pos = 0; pos < static_cast<int>(rotation.size()); ++pos) {
            const int d = rotation[pos];

            if (d < 0 || d >= static_cast<int>(prepared.darts.size())) {
                return fail("Rotation contains invalid dart id.", result);
            }

            if (prepared.darts[d].from != v) {
                std::ostringstream oss;
                oss << "Dart " << d
                    << " appears in rotation of vertex " << v
                    << ", but its source is " << prepared.darts[d].from
                    << ".";

                return fail(oss.str(), result);
            }

            if (!expectedAtVertex[d]) {
                std::ostringstream oss;
                oss << "Dart " << d
                    << " appears in rotation of vertex " << v
                    << ", but it is not in outAll[" << v << "].";

                return fail(oss.str(), result);
            }

            if (positionOfDart[d] != -1) {
                std::ostringstream oss;
                oss << "Dart " << d << " appears more than once in rotations.";

                return fail(oss.str(), result);
            }

            positionOfDart[d] = pos;
        }
    }

    for (int d = 0; d < static_cast<int>(prepared.darts.size()); ++d) {
        if (positionOfDart[d] == -1) {
            std::ostringstream oss;
            oss << "Dart " << d << " does not appear in any vertex rotation.";

            return fail(oss.str(), result);
        }

        const int r = prepared.darts[d].rev;

        if (r < 0 || r >= static_cast<int>(prepared.darts.size())) {
            return fail("Dart has invalid reverse dart.", result);
        }

        if (prepared.darts[r].rev != d) {
            return fail("Reverse dart relation is not symmetric.", result);
        }
    }

    const int faceCount = countFaces(prepared, embedding, positionOfDart, result);

    if (!result.message.empty()) {
        result.valid = false;
        return result;
    }

    result.faceCount = faceCount;

    if (result.faceCount != result.expectedFaceCount) {
        std::ostringstream oss;

        oss << "Euler face check failed: V - E + F = "
            << result.vertexCount << " - "
            << result.edgeCount << " + "
            << result.faceCount << " = "
            << (result.vertexCount - result.edgeCount + result.faceCount)
            << ", expected 2.";

        return fail(oss.str(), result);
    }

    result.valid = true;

    std::ostringstream oss;
    oss << "Embedding is valid. Euler check: V - E + F = "
        << result.vertexCount << " - "
        << result.edgeCount << " + "
        << result.faceCount << " = 2.";

    result.message = oss.str();

    return result;
}

EmbeddingValidationResult EmbeddingValidator::validateGlobalOriginalEmbedding(
    const Graph& graph,
    const Embedding& embedding
) {
    EmbeddingValidationResult result;

    result.vertexCount = graph.vertexCount();
    result.edgeCount = graph.edgeCount();
    result.faceCount = 0;
    result.expectedFaceCount = 0;

    const int n = graph.vertexCount();
    const int m = graph.edgeCount();

    if (static_cast<int>(embedding.rotationOriginalEdgeIds.size()) != n) {
        return fail(
            "Global embedding has wrong number of vertex rotations for original edge IDs.",
            result
        );
    }

    if (static_cast<int>(embedding.rotationOriginalNeighbors.size()) != n) {
        return fail(
            "Global embedding has wrong number of vertex rotations for original neighbors.",
            result
        );
    }

    std::unordered_map<int, const Edge*> edgeByOriginalId;
    edgeByOriginalId.reserve(static_cast<std::size_t>(m) * 2);

    for (const Edge& edge : graph.edges()) {
        if (edgeByOriginalId.find(edge.originalId) != edgeByOriginalId.end()) {
            return fail("Duplicate original edge ID in input graph.", result);
        }

        edgeByOriginalId[edge.originalId] = &edge;
    }

    std::unordered_map<int, int> occurrenceCount;
    occurrenceCount.reserve(static_cast<std::size_t>(m) * 2);

    for (int v = 0; v < n; ++v) {
        const auto& edgeRotation = embedding.rotationOriginalEdgeIds[v];
        const auto& neighborRotation = embedding.rotationOriginalNeighbors[v];

        if (edgeRotation.size() != neighborRotation.size()) {
            std::ostringstream oss;
            oss << "Vertex " << v
                << " has mismatching global rotation sizes: "
                << edgeRotation.size()
                << " edge IDs but "
                << neighborRotation.size()
                << " neighbors.";

            return fail(oss.str(), result);
        }

        for (std::size_t i = 0; i < edgeRotation.size(); ++i) {
            const int originalEdgeId = edgeRotation[i];
            const int neighbor = neighborRotation[i];

            auto edgeIt = edgeByOriginalId.find(originalEdgeId);

            if (edgeIt == edgeByOriginalId.end()) {
                std::ostringstream oss;
                oss << "Global embedding references unknown original edge ID "
                    << originalEdgeId
                    << ".";

                return fail(oss.str(), result);
            }

            if (neighbor < 0 || neighbor >= n) {
                std::ostringstream oss;
                oss << "Global embedding references invalid neighbor "
                    << neighbor
                    << " at vertex "
                    << v
                    << ".";

                return fail(oss.str(), result);
            }

            const Edge& edge = *edgeIt->second;

            int expectedNeighbor = -1;

            if (edge.u == v) {
                expectedNeighbor = edge.v;
            } else if (edge.v == v) {
                expectedNeighbor = edge.u;
            } else {
                std::ostringstream oss;
                oss << "Original edge ID "
                    << originalEdgeId
                    << " is listed in rotation of vertex "
                    << v
                    << ", but that edge is not incident to this vertex.";

                return fail(oss.str(), result);
            }

            if (neighbor != expectedNeighbor) {
                std::ostringstream oss;
                oss << "Wrong neighbor for original edge ID "
                    << originalEdgeId
                    << " at vertex "
                    << v
                    << ". Expected "
                    << expectedNeighbor
                    << ", got "
                    << neighbor
                    << ".";

                return fail(oss.str(), result);
            }

            occurrenceCount[originalEdgeId]++;
        }
    }

    for (const Edge& edge : graph.edges()) {
        const int count = occurrenceCount[edge.originalId];

        if (count != 2) {
            std::ostringstream oss;
            oss << "Original edge ID "
                << edge.originalId
                << " appears "
                << count
                << " times in global embedding, expected exactly 2.";

            return fail(oss.str(), result);
        }
    }

    result.valid = true;

    std::ostringstream oss;
    oss << "Global original embedding is valid: "
        << n
        << " vertices, "
        << m
        << " edges, every original edge appears exactly twice.";

    result.message = oss.str();

    return result;
}

EmbeddingValidationResult EmbeddingValidator::fail(
    const std::string& message,
    EmbeddingValidationResult result
) {
    result.valid = false;
    result.message = message;
    return result;
}

int EmbeddingValidator::countFaces(
    const PreparedPalmTree& prepared,
    const Embedding& embedding,
    const std::vector<int>& positionOfDart,
    EmbeddingValidationResult& result
) {
    const int dartCount = static_cast<int>(prepared.darts.size());

    std::vector<bool> visited(dartCount, false);

    int faces = 0;

    for (int start = 0; start < dartCount; ++start) {
        if (visited[start]) {
            continue;
        }

        int current = start;
        int guard = 0;

        while (true) {
            if (current < 0 || current >= dartCount) {
                result.message = "Face traversal reached invalid dart.";
                return -1;
            }

            if (visited[current]) {
                if (current != start) {
                    result.message = "Face traversal entered a previously visited face.";
                    return -1;
                }

                break;
            }

            visited[current] = true;
            ++guard;

            if (guard > dartCount + 1) {
                result.message = "Face traversal exceeded dart count guard.";
                return -1;
            }

            const int reverseDart = prepared.darts[current].rev;
            const int vertex = prepared.darts[reverseDart].from;
            const auto& rotation = embedding.rotationDarts[vertex];

            if (rotation.empty()) {
                result.message = "Face traversal reached vertex with empty rotation.";
                return -1;
            }

            const int reversePosition = positionOfDart[reverseDart];

            if (reversePosition < 0) {
                result.message = "Reverse dart missing from rotation system.";
                return -1;
            }

            // Enter vertex through current, then take the next dart after reverse(current).
            const int nextPosition = (reversePosition + 1) % static_cast<int>(rotation.size());
            current = rotation[nextPosition];
        }

        ++faces;
    }

    return faces;
}

} // namespace ht
