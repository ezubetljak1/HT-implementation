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
