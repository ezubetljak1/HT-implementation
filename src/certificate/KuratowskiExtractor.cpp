#include "ht/certificate/KuratowskiExtractor.hpp"

#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/WilliamsonKernelBuilder.hpp"
#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"
#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"

#include <sstream>

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

    bool usedWilliamsonKernel = false;
    bool verifiedSubdivision = false;

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
            verifier.verify(prepared, certificate.originalEdgeIds);

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
        oss << "The candidate was verified as a Kuratowski subdivision. ";
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