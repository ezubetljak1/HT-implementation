#pragma once

#include <string>
#include <vector>

namespace ht {

struct Embedding {
    // Local/component-level rotation:
    // rotationDarts[v] is the cyclic order of local directed darts leaving local vertex v.
    std::vector<std::vector<int>> rotationDarts;

    // Same local/component-level embedding translated to local neighbor vertex ids.
    std::vector<std::vector<int>> rotationNeighbors;

    // Global/original-graph rotation:
    // rotationOriginalEdgeIds[v] is the cyclic order of original edge IDs leaving original vertex v.
    std::vector<std::vector<int>> rotationOriginalEdgeIds;

    // Same global/original-graph embedding translated to original neighbor vertex IDs.
    std::vector<std::vector<int>> rotationOriginalNeighbors;
};

struct EmbeddingValidationResult {
    bool valid = false;
    std::string message;

    int vertexCount = 0;
    int edgeCount = 0;
    int faceCount = 0;
    int expectedFaceCount = 0;
};

} // namespace ht
