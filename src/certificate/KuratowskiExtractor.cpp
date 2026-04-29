#include "ht/certificate/KuratowskiExtractor.hpp"

namespace ht {

KuratowskiCertificate KuratowskiExtractor::notImplementedCertificate() const {
    KuratowskiCertificate certificate;
    certificate.type = KuratowskiType::Unknown;
    certificate.message =
        "Kuratowski extraction is not implemented yet. "
        "The next step is to preserve a strong-planarity failure witness and trace original edge IDs.";
    return certificate;
}

KuratowskiCertificate KuratowskiExtractor::extractFromFailure(
    const PreparedPalmTree& /* prepared */
) const {
    return notImplementedCertificate();
}

} // namespace ht
