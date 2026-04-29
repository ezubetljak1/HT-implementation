#include "TestSupport.hpp"

#include "ht/certificate/KuratowskiExtractor.hpp"

using namespace ht;

HT_TEST(KuratowskiExtractorPlaceholderIsExplicit) {
    KuratowskiExtractor extractor;
    KuratowskiCertificate certificate = extractor.notImplementedCertificate();

    assert(certificate.type == KuratowskiType::Unknown);
    assert(certificate.originalEdgeIds.empty());
    assert(!certificate.message.empty());
}
