#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/embedding/EmbeddingValidator.hpp"
#include "ht/embedding/HTEmbeddingBuilder.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

using namespace ht;

HT_TEST(EmbeddingBuilderAndValidatorRunOnTriangle) {
    Graph g = ht::test::buildTriangle();

    BiconnectedComponentsFinder finder;
    Components components = finder.find(g);

    ComponentPreprocessor preprocessor;
    PreprocessedComponent pc = preprocessor.preprocess(components[0]);

    PreparedPalmTreeBuilder builder;
    PreparedPalmTree prepared = builder.build(pc);

    StrongPlanarityTester strong(prepared, prepared.number);
    std::vector<Side> alpha;

    assert(strong.run(prepared.rootTreeDart, alpha));

    prepared.alpha = alpha;

    HTEmbeddingBuilder embeddingBuilder(prepared);
    Embedding embedding = embeddingBuilder.run();

    EmbeddingValidationResult validation =
        EmbeddingValidator::validate(prepared, embedding);

    assert(validation.valid);
}
