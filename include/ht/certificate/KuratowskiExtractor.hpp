#pragma once

#include "ht/certificate/KuratowskiCertificate.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

namespace ht {

class KuratowskiExtractor {
public:
    KuratowskiCertificate notImplementedCertificate() const;

    // Placeholder for future real extraction from a strong-planarity failure witness.
    KuratowskiCertificate extractFromFailure(const PreparedPalmTree& prepared) const;

    KuratowskiCertificate extractFromFailure(
        const PreparedPalmTree& prepared,
        const StrongPlanarityFailure& failure
    ) const;
};

} // namespace ht
