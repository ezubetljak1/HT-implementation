#include "ht/certificate/KuratowskiExtractor.hpp"

#include <sstream>

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

KuratowskiCertificate KuratowskiExtractor::extractFromFailure(
    const PreparedPalmTree& prepared,
    const StrongPlanarityFailure& failure
) const {
    KuratowskiCertificate certificate;
    certificate.type = KuratowskiType::Unknown;

    std::ostringstream oss;

    oss << "Kuratowski extraction is not implemented yet, "
        << "but strong-planarity produced a failure witness. ";

    if (!failure.hasFailure()) {
        oss << "No structured failure data was recorded.";
        certificate.message = oss.str();
        return certificate;
    }

    oss << "Failure type = ";

    switch (failure.type) {
        case StrongPlanarityFailureType::UnresolvableLeftInterlace:
            oss << "UnresolvableLeftInterlace";
            break;
        case StrongPlanarityFailureType::BothSidesAttachAboveW0:
            oss << "BothSidesAttachAboveW0";
            break;
        case StrongPlanarityFailureType::None:
            oss << "None";
            break;
    }

    oss << ", rootTreeDart = " << failure.rootTreeDart
        << ", cycleRootDart = " << failure.cycleRootDart
        << ", currentDart = " << failure.currentDart
        << ", cycle vertices: x=" << failure.x
        << ", y=" << failure.y
        << ", w0=" << failure.w0
        << ", wk=" << failure.wk
        << ", closingBackDart=" << failure.closingBackDart
        << ", prepared graph has " << prepared.n
        << " vertices and " << prepared.edgeCount
        << " edges.";

    certificate.message = oss.str();
    return certificate;
}

} // namespace ht
