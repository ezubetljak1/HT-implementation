#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ht/Graph.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/HomKernelSignatureBuilder.hpp"
#include "ht/certificate/KuratowskiKernelSelector.hpp"
#include "ht/certificate/KuratowskiSubdivisionVerifier.hpp"
#include "ht/certificate/PathTreeBuilder.hpp"
#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonContextBuilder.hpp"
#include "ht/certificate/WilliamsonKernelBuilder.hpp"
#include "ht/certificate/WilliamsonSegmentListBuilder.hpp"
#include "ht/certificate/WilliamsonSegfoPathBuilder.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"
#include "ht/certificate/HomKernelCanonicalizer.hpp"

#include "TestGraphs.hpp"

using namespace ht;

namespace {

struct PipelineOutput {
    bool valid = false;

    PreparedPalmTree prepared;
    WilliamsonContext context;
    WilliamsonSegmentList segmentList;
    WilliamsonSegfoPath segfoPath;
    WilliamsonKernel kernel;
    KuratowskiSubdivisionVerification selected;

    std::string message;
};

std::string certificateTypeToString(KuratowskiType type) {
    switch (type) {
        case KuratowskiType::Unknown:
            return "Unknown";
        case KuratowskiType::K5Subdivision:
            return "K5Subdivision";
        case KuratowskiType::K33Subdivision:
            return "K33Subdivision";
    }

    return "Unknown";
}

PipelineOutput runWilliamsonPipeline(const Graph& graph) {
    PipelineOutput output;

    BiconnectedComponentsFinder finder;
    Components components = finder.find(graph);

    ComponentPreprocessor preprocessor;
    PreparedPalmTreeBuilder preparedBuilder;

    for (const Component& component : components) {
        if (component.size() <= 1) {
            continue;
        }

        PreprocessedComponent preprocessed =
            preprocessor.preprocess(component);

        PreparedPalmTree prepared =
            preparedBuilder.build(preprocessed);

        if (prepared.rootTreeDart == -1) {
            continue;
        }

        StrongPlanarityTester strongTester(prepared, prepared.number);
        std::vector<Side> alpha;

        const bool planar =
            strongTester.run(
                prepared.rootTreeDart,
                alpha
            );

        if (planar) {
            continue;
        }

        const StrongPlanarityFailure& failure =
            strongTester.failure();

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

        if (!context.valid) {
            output.message = "Williamson context is invalid.";
            return output;
        }

        WilliamsonSegmentListBuilder segmentListBuilder;
        WilliamsonSegmentList segmentList =
            segmentListBuilder.build(
                prepared,
                pathTree,
                context
            );

        if (!segmentList.valid) {
            output.message = "Williamson segment list is invalid.";
            return output;
        }

        WilliamsonSegfoPathBuilder segfoPathBuilder;
        WilliamsonSegfoPath segfoPath =
            segfoPathBuilder.buildPath(
                prepared,
                pathTree,
                metadata,
                segmentList,
                context
            );

        if (!segfoPath.valid) {
            output.message = "Williamson SEGFO path is invalid.";
            return output;
        }

        WilliamsonKernelBuilder kernelBuilder;
        WilliamsonKernel kernel =
            kernelBuilder.buildKernelFromSegfoPath(
                prepared,
                pathTree,
                context,
                segfoPath
            );

        if (!kernel.valid) {
            output.message = "Williamson kernel is invalid.";
            return output;
        }

        KuratowskiKernelSelector selector;
        KuratowskiSubdivisionVerification selected =
            selector.select(
                prepared,
                kernel.originalEdgeIds
            );

        output.valid = true;
        output.prepared = prepared;
        output.context = context;
        output.segmentList = segmentList;
        output.segfoPath = segfoPath;
        output.kernel = kernel;
        output.selected = selected;
        output.message = "Williamson pipeline succeeded.";
        return output;
    }

    output.message =
        "No non-planar biconnected component was found.";
    return output;
}

int maxEdgeId(
    const HomKernelSignature& signature,
    const KuratowskiSubdivisionVerification& selected
) {
    int maxId = -1;

    for (const HomKernelSkeletonEdge& skeletonEdge : signature.skeletonEdges) {
        for (int edgeId : skeletonEdge.originalEdgeIds) {
            if (edgeId > maxId) {
                maxId = edgeId;
            }
        }
    }

    for (int edgeId : selected.originalEdgeIds) {
        if (edgeId > maxId) {
            maxId = edgeId;
        }
    }

    return maxId;
}

bool skeletonEdgeIsSelected(
    const HomKernelSkeletonEdge& skeletonEdge,
    const std::vector<char>& selectedEdge
) {
    if (skeletonEdge.originalEdgeIds.empty()) {
        return false;
    }

    for (int edgeId : skeletonEdge.originalEdgeIds) {
        if (edgeId < 0 || edgeId >= static_cast<int>(selectedEdge.size())) {
            return false;
        }

        if (!selectedEdge[edgeId]) {
            return false;
        }
    }

    return true;
}

void inspectGraph(
    std::ostream& out,
    const std::string& name,
    const Graph& graph
) {
    out << "========================================\n";
    out << "Graph: " << name << "\n";
    out << "V = " << graph.vertexCount()
        << ", E = " << graph.edgeCount()
        << "\n";

    PipelineOutput pipeline =
        runWilliamsonPipeline(graph);

    if (!pipeline.valid) {
        out << "Pipeline failed: "
            << pipeline.message
            << "\n\n";
        return;
    }

    HomKernelSignatureBuilder signatureBuilder;
    HomKernelSignature signature =
        signatureBuilder.build(
            pipeline.prepared,
            pipeline.kernel.originalEdgeIds
        );

    if (!signature.valid) {
        out << "Signature failed: "
            << signature.message
            << "\n\n";
        return;
    }

    HomKernelCanonicalizer canonicalizer;
    CanonicalHomKernelSignature canonical =
        canonicalizer.canonicalize(signature);

    if (!canonical.valid) {
        out << "Canonical signature failed: "
            << canonical.message
            << "\n\n";
        return;
    }

    out << "Shape key:\n";
    out << signature.shapeKey << "\n\n";

    out << "Canonical key:\n";
    out << canonical.key << "\n\n";

    out << "Canonical edge -> skeleton edge mapping:\n";
    for (int i = 0;
        i < static_cast<int>(canonical.canonicalEdgeToSkeletonEdge.size());
        ++i) {
        out << "  canonical edge " << i
            << " -> skeleton edge "
            << canonical.canonicalEdgeToSkeletonEdge[i]
            << "\n";
    }
    out << "\n";

    out << "Canonical vertex -> skeleton vertex mapping:\n";
    for (int i = 0;
        i < static_cast<int>(canonical.canonicalVertexToSkeletonVertex.size());
        ++i) {
        out << "  canonical vertex " << i
            << " -> skeleton vertex "
            << canonical.canonicalVertexToSkeletonVertex[i]
            << "\n";
    }
    out << "\n";

    out << "Context nodes:\n";
    out << "  fNode = " << pipeline.context.fNode << "\n";
    out << "  aNode = " << pipeline.context.aNode << "\n";
    out << "  bNode = " << pipeline.context.bNode << "\n";
    out << "  cycleNode = " << pipeline.context.cycleNode << "\n\n";

    out << "SEGFO path nodes: ";
    for (int nodeId : pipeline.segfoPath.segmentPathNodes) {
        out << nodeId << " ";
    }
    out << "\n\n";

    out << "Kernel originalEdgeIds [size="
        << pipeline.kernel.originalEdgeIds.size()
        << "]: ";

    for (int edgeId : pipeline.kernel.originalEdgeIds) {
        out << edgeId << " ";
    }
    out << "\n\n";

    out << "Selected certificate:\n";
    out << "  type = "
        << certificateTypeToString(pipeline.selected.type)
        << "\n";
    out << "  originalEdgeIds [size="
        << pipeline.selected.originalEdgeIds.size()
        << "]: ";

    for (int edgeId : pipeline.selected.originalEdgeIds) {
        out << edgeId << " ";
    }
    out << "\n\n";

    const int maxId =
        maxEdgeId(
            signature,
            pipeline.selected
        );

    std::vector<char> selectedEdge(
        static_cast<std::size_t>(maxId + 1),
        0
    );

    for (int edgeId : pipeline.selected.originalEdgeIds) {
        if (edgeId >= 0 && edgeId <= maxId) {
            selectedEdge[edgeId] = 1;
        }
    }

    out << "HOMKERNEL skeleton vertices:\n";
    for (int i = 0;
         i < static_cast<int>(signature.skeletonVertexToPreparedVertex.size());
         ++i) {
        out << "  skeleton vertex " << i
            << " -> prepared vertex "
            << signature.skeletonVertexToPreparedVertex[i]
            << "\n";
    }
    out << "\n";

    out << "HOMKERNEL skeleton edges:\n";
    for (const HomKernelSkeletonEdge& skeletonEdge : signature.skeletonEdges) {
        const bool selected =
            skeletonEdgeIsSelected(
                skeletonEdge,
                selectedEdge
            );

        out << "  [" << skeletonEdge.id << "] "
            << skeletonEdge.u << " -- " << skeletonEdge.v
            << " pathOriginalEdgeIds=";

        for (int edgeId : skeletonEdge.originalEdgeIds) {
            out << edgeId << " ";
        }

        out << (selected ? " SELECTED" : "")
            << "\n";
    }

    out << "\n";
}

void writeReport(std::ostream& out) {
    out << "HOMKERNEL Signature Report\n";
    out << "========================================\n\n";

    inspectGraph(out, "K5", ht::test::buildK5());
    inspectGraph(out, "K3,3", ht::test::buildK33());
    inspectGraph(out, "Subdivided K5", ht::test::buildSubdividedK5());
    inspectGraph(out, "Subdivided K3,3", ht::test::buildSubdividedK33());
    inspectGraph(out, "Partially subdivided K5", ht::test::buildPartiallySubdividedK5());
    inspectGraph(out, "Partially subdivided K3,3", ht::test::buildPartiallySubdividedK33());
    inspectGraph(out, "Petersen graph", ht::test::buildPetersenGraph());
    inspectGraph(out, "DM Rijeseni 14b", ht::test::buildDMRijeseni14b());
    inspectGraph(out, "DM Rijeseni 15", ht::test::buildDMRijeseni15());
    inspectGraph(out, "DM zsr 10", ht::test::buildDMzsr10());
}

} // namespace

int main(int argc, char** argv) {
    std::string outputPath = "homkernel_signature_report.txt";

    if (argc >= 2) {
        outputPath = argv[1];
    }

    std::ofstream output(outputPath);

    if (!output) {
        std::cerr << "Could not open output file: "
                  << outputPath
                  << "\n";
        return 1;
    }

    writeReport(output);

    std::cout << "HOMKERNEL signature report written to: "
              << outputPath
              << "\n";

    return 0;
}