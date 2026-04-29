#pragma once

#include <vector>

#include "ht/embedding/Embedding.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class EmbeddingValidator {
public:
    static EmbeddingValidationResult validate(
        const PreparedPalmTree& prepared,
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
