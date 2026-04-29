#pragma once

#include <string>
#include <vector>

namespace ht {

struct Embedding {
    // rotationDarts[v] is the cyclic clockwise order of directed darts leaving v.
    std::vector<std::vector<int>> rotationDarts;

    // Same embedding translated to local neighbor vertex ids.
    std::vector<std::vector<int>> rotationNeighbors;
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
