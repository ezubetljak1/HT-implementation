#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

using namespace ht;

HT_TEST(StrongPlanarityRunsOnTriangle) {
    Graph g = ht::test::buildTriangle();

    BiconnectedComponentsFinder finder;
    Components components = finder.find(g);

    ComponentPreprocessor preprocessor;
    PreprocessedComponent pc = preprocessor.preprocess(components[0]);

    PreparedPalmTreeBuilder builder;
    PreparedPalmTree prepared = builder.build(pc);

    assert(prepared.rootTreeDart != -1);

    StrongPlanarityTester tester(prepared, prepared.number);
    std::vector<Side> alpha;

    bool planar = tester.run(prepared.rootTreeDart, alpha);

    assert(planar);
    assert(alpha.size() == prepared.darts.size());
}

HT_TEST(StrongPlanarityRecordsFailureForK33) {
    Graph g = ht::test::buildK33();

    BiconnectedComponentsFinder finder;
    Components components = finder.find(g);

    assert(components.size() == 1);

    ComponentPreprocessor preprocessor;
    PreprocessedComponent pc = preprocessor.preprocess(components[0]);

    PreparedPalmTreeBuilder builder;
    PreparedPalmTree prepared = builder.build(pc);

    assert(prepared.rootTreeDart != -1);

    StrongPlanarityTester tester(prepared, prepared.number);
    std::vector<Side> alpha;

    bool planar = tester.run(prepared.rootTreeDart, alpha);

    assert(!planar);

    const StrongPlanarityFailure& failure = tester.failure();

    assert(failure.hasFailure());
    assert(failure.type != StrongPlanarityFailureType::None);
    assert(failure.rootTreeDart == prepared.rootTreeDart);
    assert(failure.cycleRootDart != -1);
    assert(!failure.cycleSpineDarts.empty());
    assert(!failure.cycleTreeDarts.empty());
    assert(failure.cycleTreeDarts.size() >= failure.cycleSpineDarts.size());
    assert(!failure.cycleEmanatingDarts.empty());
    assert(!failure.cycleRootEmanatingDarts.empty());
    assert(!failure.message.empty());
}