#include "ht/PlanarityTester.hpp"

#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/KuratowskiExtractor.hpp"
#include "ht/embedding/EmbeddingValidator.hpp"
#include "ht/embedding/HTEmbeddingBuilder.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

namespace ht {

PlanarityResult PlanarityTester::test(const Graph& graph, bool buildEmbedding) const {
    PlanarityResult result;

    const int n = graph.vertexCount();
    const int m = graph.edgeCount();

    Embedding globalEmbedding;

    if (buildEmbedding) {
        globalEmbedding.rotationOriginalEdgeIds.resize(n);
        globalEmbedding.rotationOriginalNeighbors.resize(n);
    }

    if (n <= 2) {
        result.planar = true;
        result.message = "Graph with at most two vertices is planar.";
        return result;
    }

    if (m > 3 * n - 6) {
        result.planar = false;
        result.certificate = KuratowskiExtractor().notImplementedCertificate();
        result.message =
            "Graph violates the planar edge bound m <= 3n - 6. "
            "Kuratowski extraction is not implemented yet.";
        return result;
    }

    BiconnectedComponentsFinder bccFinder;
    Components components = bccFinder.find(graph);

    ComponentPreprocessor preprocessor;
    PreparedPalmTreeBuilder preparedBuilder;

    for (const Component& component : components) {
        if (component.empty()) {
            continue;
        }

        if (component.size() == 1) {
            if (buildEmbedding) {
                const Edge& edge = component.front();

                globalEmbedding.rotationOriginalEdgeIds[edge.u].push_back(edge.originalId);
                globalEmbedding.rotationOriginalNeighbors[edge.u].push_back(edge.v);

                globalEmbedding.rotationOriginalEdgeIds[edge.v].push_back(edge.originalId);
                globalEmbedding.rotationOriginalNeighbors[edge.v].push_back(edge.u);
            }

            continue;
        }

        PreprocessedComponent preprocessed = preprocessor.preprocess(component);
        PreparedPalmTree prepared = preparedBuilder.build(preprocessed);

        if (prepared.rootTreeDart == -1) {
            continue;
        }

        StrongPlanarityTester strongTester(prepared, prepared.number);
        std::vector<Side> alpha;

        if (!strongTester.run(prepared.rootTreeDart, alpha)) {
            result.planar = false;
            result.certificate = KuratowskiExtractor().extractFromFailure(prepared);
            result.message =
                "Strong-planarity phase reported non-planarity. "
                "Certificate extraction is still a placeholder.";
            return result;
        }

        prepared.alpha = alpha;

        if (buildEmbedding) {
            HTEmbeddingBuilder embeddingBuilder(prepared);
            Embedding embedding = embeddingBuilder.run();

            EmbeddingValidationResult validation =
                EmbeddingValidator::validate(prepared, embedding);

            if (!validation.valid) {
                result.planar = false;
                result.message =
                    "Embedding builder produced an invalid embedding: "
                    + validation.message;
                return result;
            }

            for (int localVertex = 0; localVertex < prepared.n; ++localVertex) {
            const int originalVertex = prepared.localToOriginal[localVertex];

            globalEmbedding.rotationOriginalEdgeIds[originalVertex].insert(
                globalEmbedding.rotationOriginalEdgeIds[originalVertex].end(),
                embedding.rotationOriginalEdgeIds[localVertex].begin(),
                embedding.rotationOriginalEdgeIds[localVertex].end()
            );

            globalEmbedding.rotationOriginalNeighbors[originalVertex].insert(
                globalEmbedding.rotationOriginalNeighbors[originalVertex].end(),
                embedding.rotationOriginalNeighbors[localVertex].begin(),
                embedding.rotationOriginalNeighbors[localVertex].end()
            );
        }
        }
    }

    if (buildEmbedding) {
        EmbeddingValidationResult globalValidation =
            EmbeddingValidator::validateGlobalOriginalEmbedding(graph, globalEmbedding);

        if (!globalValidation.valid) {
            result.planar = false;
            result.message =
                "Global embedding assembly failed validation: "
                + globalValidation.message;
            return result;
        }

        result.embedding = globalEmbedding;
    }

    result.planar = true;
    result.message =
    "All biconnected components passed the current HT strong-planarity pipeline. "
    "A global original-vertex embedding rotation was assembled by concatenating block rotations. "
    "Kuratowski extraction remains future work.";

    return result;
}

} // namespace ht
