#include "TestSupport.hpp"

#include "TestGraphs.hpp"
#include "ht/PlanarityTester.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/KuratowskiCertificate.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"

using namespace ht;

HT_TEST(KuratowskiExtractorVerifiesK33Subdivision) {
    Graph g = ht::test::buildK33();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, false);

    assert(!result.planar);
    assert(!result.certificate.originalEdgeIds.empty());

    assert(result.certificate.type == KuratowskiType::K33Subdivision);
    assert(result.certificate.originalEdgeIds.size() == 9);
}

HT_TEST(KuratowskiExtractorVerifiesSubdividedK5Subdivision) {
    Graph g = ht::test::buildSubdividedK5();

    PlanarityTester tester;
    PlanarityResult result = tester.test(g, false);

    assert(!result.planar);
    assert(!result.certificate.originalEdgeIds.empty());

    assert(result.certificate.type == KuratowskiType::K5Subdivision);
    assert(result.certificate.originalEdgeIds.size() == 20);
}