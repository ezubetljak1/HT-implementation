#include "ht/certificate/KuratowskiExtractor.hpp"
#include "ht/certificate/KuratowskiCandidateBuilder.hpp"

#include <sstream>

namespace ht {
int KuratowskiExtractor::maxOriginalEdgeId(const PreparedPalmTree& prepared) {
    int maxId = -1;

    for (const Dart& dart : prepared.darts) {
        if (dart.originalEdgeId > maxId) {
            maxId = dart.originalEdgeId;
        }
    }

    return maxId;
}

void KuratowskiExtractor::addUniqueOriginalEdgeFromDart(
    const PreparedPalmTree& prepared,
    int dartId,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        return;
    }

    const int originalEdgeId = prepared.darts[dartId].originalEdgeId;

    if (originalEdgeId < 0 || originalEdgeId >= static_cast<int>(seen.size())) {
        return;
    }

    if (seen[originalEdgeId]) {
        return;
    }

    seen[originalEdgeId] = 1;
    originalEdgeIds.push_back(originalEdgeId);
}

void KuratowskiExtractor::addUniqueOriginalEdgesFromDarts(
    const PreparedPalmTree& prepared,
    const std::vector<int>& dartIds,
    std::vector<char>& seen,
    std::vector<int>& originalEdgeIds
) {
    for (int dartId : dartIds) {
        addUniqueOriginalEdgeFromDart(prepared, dartId, seen, originalEdgeIds);
    }
}

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

    KuratowskiCandidateBuilder candidateBuilder;
    certificate.originalEdgeIds =
        candidateBuilder.buildOriginalEdgeCandidate(prepared, failure);

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
        << ", cycleSpineDartCount=" << failure.cycleSpineDarts.size()
        << ", cycleStemDartCount=" << failure.cycleStemDarts.size()
        << ", cycleTreeDartCount=" << failure.cycleTreeDarts.size()
        << ", cycleEmanatingDartCount=" << failure.cycleEmanatingDarts.size()
        << ", cycleRootEmanatingDartCount=" << failure.cycleRootEmanatingDarts.size()
        << ", prepared graph has " << prepared.n
        << " vertices and " << prepared.edgeCount
        << " edges.";

     oss << " Candidate original edge count = "
        << certificate.originalEdgeIds.size()
        << ". These edges are only a failure-witness candidate set, not yet a verified Kuratowski subdivision.";

    certificate.message = oss.str();
    return certificate;
}

} // namespace ht
