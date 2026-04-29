#include "ht/certificate/KuratowskiExtractor.hpp"

#include "ht/certificate/KuratowskiCandidateBuilder.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonBasicCaseDetector.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/WilliamsonKernelBuilder.hpp"
#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"
#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"
#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"

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
        addUniqueOriginalEdgeFromDart(
            prepared,
            dartId,
            seen,
            originalEdgeIds
        );
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

    bool usedWilliamsonKernel = false;

    if (failure.hasFailure()) {
        PathTreeBuilder pathTreeBuilder;
        PathTree pathTree = pathTreeBuilder.build(prepared);

        SegmentMetadataBuilder metadataBuilder;
        SegmentMetadataTable metadata =
            metadataBuilder.build(prepared, pathTree);

        WilliamsonContextBuilder contextBuilder;
        WilliamsonContext context =
            contextBuilder.build(
                prepared,
                pathTree,
                metadata,
                failure
            );

        if (context.valid) {
            WilliamsonKernelBuilder kernelBuilder;
            WilliamsonKernel kernel;

            // Preferred Williamson route:
            // SEGLIST(e) -> SEGFO path B -> ... -> A -> kernel.
            WilliamsonSegmentListBuilder segmentListBuilder;
            WilliamsonSegmentList segmentList =
                segmentListBuilder.build(
                    prepared,
                    pathTree,
                    context
                );

            if (segmentList.valid) {
                WilliamsonSegfoPathBuilder segfoPathBuilder;
                WilliamsonSegfoPath segfoPath =
                    segfoPathBuilder.buildPath(
                        prepared,
                        pathTree,
                        metadata,
                        segmentList,
                        context
                    );

                if (segfoPath.valid) {
                    kernel =
                        kernelBuilder.buildKernelFromSegfoPath(
                            prepared,
                            pathTree,
                            context,
                            segfoPath
                        );
                }
            }

            // Fallback only if the general SEGFO path was not found.
            // This covers direct Williamson basic case 1: F dl B dl A dl F.
            if (!kernel.valid) {
                WilliamsonBasicCaseDetector basicCaseDetector;
                WilliamsonBasicCase basicCase =
                    basicCaseDetector.detect(
                        prepared,
                        pathTree,
                        metadata,
                        context
                    );

                if (basicCase.valid) {
                    kernel =
                        kernelBuilder.buildDirectKernel(
                            prepared,
                            pathTree,
                            basicCase
                        );
                }
            }

            if (kernel.valid && !kernel.originalEdgeIds.empty()) {
                certificate.originalEdgeIds = kernel.originalEdgeIds;
                usedWilliamsonKernel = true;
            }
        }
    }

    if (!usedWilliamsonKernel) {
        KuratowskiCandidateBuilder candidateBuilder;
        certificate.originalEdgeIds =
            candidateBuilder.buildOriginalEdgeCandidate(
                prepared,
                failure
            );
    }

    KuratowskiSubdivisionVerifier verifier;
        KuratowskiSubdivisionVerification verification =
            verifier.verify(
                prepared,
                certificate.originalEdgeIds
            );

        bool verifiedSubdivision = false;

        if (verification.valid) {
            certificate.type = verification.type;
            certificate.originalEdgeIds = verification.originalEdgeIds;
            verifiedSubdivision = true;
        }

    std::ostringstream oss;

    oss << "Kuratowski extraction is not fully implemented yet, "
        << "but strong-planarity produced a failure witness. ";

    if (usedWilliamsonKernel) {
        oss << "A Williamson kernel candidate was constructed. ";
    } else {
        oss << "Falling back to the older failure-witness candidate builder. ";
    }

    if (verifiedSubdivision) {
        oss << "The candidate was verified as a Kuratowski subdivision. ";
    } else {
        oss << "The candidate was not yet verified as a Kuratowski subdivision. ";
    }

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
        << ". These edges are a Kuratowski/kernel candidate; final subdivision verification is not implemented yet.";

    certificate.message = oss.str();
    return certificate;
}

} // namespace ht