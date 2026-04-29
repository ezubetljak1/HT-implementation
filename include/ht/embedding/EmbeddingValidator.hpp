#pragma once

#include <vector>

#include "ht/embedding/Embedding.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"
#include "ht/Graph.hpp"

namespace ht {

class EmbeddingValidator {
public:
    static EmbeddingValidationResult validate(
        const PreparedPalmTree& prepared,
        const Embedding& embedding
    );

    static EmbeddingValidationResult validateGlobalOriginalEmbedding(
        const Graph& graph,
        const Embedding& embedding
    );

private:
    static EmbeddingValidationResult fail(
        const std::string& message,
        EmbeddingValidationResult result
    );

    static int countFaces(
        const PreparedPalmTree& prepared,
        const Embedding& embedding,
        const std::vector<int>& positionOfDart,
        EmbeddingValidationResult& result
    );
};

} // namespace ht
