#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"

using namespace ht;

HT_TEST(ComponentPreprocessorComputesNumbersAndLowpoints) {
    Graph g = ht::test::buildTriangle();

    BiconnectedComponentsFinder finder;
    Components components = finder.find(g);

    assert(components.size() == 1);

    ComponentPreprocessor preprocessor;
    PreprocessedComponent pc = preprocessor.preprocess(components[0]);

    assert(pc.context.localGraph.vertexCount() == 3);
    assert(pc.number.size() == 3);
    assert(pc.lowpt1.size() == 3);
    assert(pc.lowpt2.size() == 3);
    assert(pc.orderedOut.size() == 3);

    for (int n : pc.number) {
        assert(n >= 1);
        assert(n <= 3);
    }
}
