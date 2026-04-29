#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"

using namespace ht;

HT_TEST(PreparedPalmTreeBuilderCreatesDarts) {
    Graph g = ht::test::buildTriangle();

    BiconnectedComponentsFinder finder;
    Components components = finder.find(g);

    ComponentPreprocessor preprocessor;
    PreprocessedComponent pc = preprocessor.preprocess(components[0]);

    PreparedPalmTreeBuilder builder;
    PreparedPalmTree prepared = builder.build(pc);

    assert(prepared.n == 3);
    assert(prepared.edgeCount == 3);
    assert(prepared.darts.size() == 6);
    assert(prepared.outAll.size() == 3);

    for (const Dart& d : prepared.darts) {
        assert(d.rev >= 0);
        assert(d.rev < static_cast<int>(prepared.darts.size()));
        assert(prepared.darts[d.rev].rev == d.id);
    }
}
