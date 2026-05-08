#include "ht/certificate/KuratowskiExtractor.hpp"

#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/WilliamsonKernelBuilder.hpp"
#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"
#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"
#include "ht/certificate/KuratowskiKernelSelector.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<int> collectAllPreparedOriginalEdgeIds(
    const ht::PreparedPalmTree& prepared
) {
    int maxOriginalEdgeId = -1;

    for (const ht::Dart& dart : prepared.darts) {
        if (dart.originalEdgeId > maxOriginalEdgeId) {
            maxOriginalEdgeId = dart.originalEdgeId;
        }
    }

    if (maxOriginalEdgeId < 0) {
        return {};
    }

    std::vector<char> seen(
        static_cast<std::size_t>(maxOriginalEdgeId + 1),
        0
    );

    std::vector<int> originalEdgeIds;

    for (const ht::Dart& dart : prepared.darts) {
        const int originalEdgeId = dart.originalEdgeId;

        if (originalEdgeId < 0 ||
            originalEdgeId >= static_cast<int>(seen.size())) {
            continue;
        }

        if (seen[originalEdgeId]) {
            continue;
        }

        seen[originalEdgeId] = 1;
        originalEdgeIds.push_back(originalEdgeId);
    }

    return originalEdgeIds;
}

ht::KuratowskiCertificate makeCertificateFromVerification(
    const ht::KuratowskiSubdivisionVerification& verification,
    const std::string& prefixMessage
) {
    ht::KuratowskiCertificate certificate;

    if (!verification.valid) {
        certificate.type = ht::KuratowskiType::Unknown;
        certificate.message = prefixMessage + verification.message;
        return certificate;
    }

    certificate.type = verification.type;
    certificate.originalEdgeIds = verification.originalEdgeIds;
    certificate.message = prefixMessage + verification.message;

    return certificate;
}

} // namespace

namespace ht {

KuratowskiCertificate KuratowskiExtractor::notImplementedCertificate() const {
    KuratowskiCertificate certificate;
    certificate.type = KuratowskiType::Unknown;
    certificate.message =
        "Kuratowski extraction is not implemented for this case yet.";

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

    {
        const std::vector<int> wholeComponentCandidate =
            collectAllPreparedOriginalEdgeIds(prepared);

        KuratowskiSubdivisionVerifier verifier;

        const KuratowskiSubdivisionVerification verification =
            verifier.verify(
                prepared,
                wholeComponentCandidate
            );

        if (verification.valid) {
            return makeCertificateFromVerification(
                verification,
                "The entire prepared component is already a verified "
                "Kuratowski subdivision. "
            );
        }
    }

    bool usedWilliamsonKernel = false;
    bool verifiedSubdivision = false;

    std::string contextMessage;
    std::string segmentListMessage;
    std::string segfoPathMessage;
    std::string kernelMessage;

    if (failure.hasFailure()) {
        PathTreeBuilder pathTreeBuilder;
        PathTree pathTree =
            pathTreeBuilder.build(prepared);

        SegmentMetadataBuilder metadataBuilder;
        SegmentMetadataTable metadata =
            metadataBuilder.build(
                prepared,
                pathTree
            );

        WilliamsonContextBuilder contextBuilder;
        WilliamsonContext context =
            contextBuilder.build(
                prepared,
                pathTree,
                metadata,
                failure
            );

        contextMessage = context.message;

        if (context.valid) {
            WilliamsonKernelBuilder kernelBuilder;
            WilliamsonKernel kernel;

            WilliamsonSegmentListBuilder segmentListBuilder;
            WilliamsonSegmentList segmentList =
                segmentListBuilder.build(
                    prepared,
                    pathTree,
                    context
                );
            
            segmentListMessage = segmentList.message;

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
                
                segfoPathMessage = segfoPath.message;

                if (segfoPath.valid) {
                    kernel =
                        kernelBuilder.buildKernelFromSegfoPath(
                            prepared,
                            pathTree,
                            metadata,
                            context,
                            segfoPath
                        );
                    
                    kernelMessage = kernel.message;
                }
            }

            if (kernel.valid && !kernel.originalEdgeIds.empty()) {
                certificate.originalEdgeIds =
                    kernel.originalEdgeIds;
                usedWilliamsonKernel = true;
            }
        }
    }


    if (!certificate.originalEdgeIds.empty()) {
        KuratowskiSubdivisionVerifier verifier;

        KuratowskiSubdivisionVerification verification =
            verifier.verify(
                prepared,
                certificate.originalEdgeIds
            );

        // This is now legitimate Williamson HOMKERNEL selection.
        //
        // Important:
        // We only call the selector after a path-only Williamson kernel was built.
        // We are NOT using it as a fallback for the old full-subtree SEG union.
        if (!verification.valid && usedWilliamsonKernel) {
            KuratowskiKernelSelector selector;

            verification =
                selector.select(
                    prepared,
                    certificate.originalEdgeIds
                );
        }

        if (verification.valid) {
            certificate.type = verification.type;
            certificate.originalEdgeIds = verification.originalEdgeIds;
            verifiedSubdivision = true;
        }
    }

    std::ostringstream oss;

    if (usedWilliamsonKernel) {
        oss << "A Williamson SEGFO/kernel candidate was constructed. ";
    } else {
        oss << "Williamson pipeline did not produce a kernel candidate. ";
    }

    if (verifiedSubdivision) {
        oss << "The candidate was verified/selected as a Kuratowski subdivision. ";
    } else if (!certificate.originalEdgeIds.empty()) {
        oss << "The candidate was not verified as a Kuratowski subdivision. ";
    } else {
        oss << "No Kuratowski candidate edge set was produced. ";
    }

    if (!failure.hasFailure()) {
        oss << "No structured strong-planarity failure data was recorded.";
        certificate.message = oss.str();
        return certificate;
    }

    oss << "Williamson context: " << contextMessage << ". ";
    oss << "SEGLIST: " << segmentListMessage << ". ";
    oss << "SEGFO path: " << segfoPathMessage << ". ";
    oss << "Kernel: " << kernelMessage << ". ";

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
        << ", cycleRootEmanatingDartCount="
        << failure.cycleRootEmanatingDarts.size()
        << ", prepared graph has " << prepared.n
        << " vertices and " << prepared.edgeCount
        << " edges.";

    oss << " Certificate original edge count = "
        << certificate.originalEdgeIds.size()
        << ".";

    certificate.message = oss.str();
    return certificate;
}

} // namespace ht