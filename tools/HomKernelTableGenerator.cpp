#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "ht/Graph.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/certificate/HomKernelCanonicalizer.hpp"
#include "ht/certificate/HomKernelSignatureBuilder.hpp"
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

#include "TestGraphs.hpp"

using namespace ht;

namespace {

struct NamedGraph {
    std::string name;
    Graph graph;
};

struct PipelineOutput {
    bool valid = false;

    PreparedPalmTree prepared;
    WilliamsonContext context;
    WilliamsonKernel kernel;

    std::string message;
};

struct GeneratedEntry {
    std::string graphName;

    std::string canonicalKey;
    std::string roleAwareKey;

    int canonicalFEdge = -1;
    int canonicalAEdge = -1;
    int canonicalBEdge = -1;

    std::vector<int> selectedCanonicalEdgeIds;

    KuratowskiType type = KuratowskiType::Unknown;
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

        output.valid = true;
        output.prepared = prepared;
        output.context = context;
        output.kernel = kernel;
        output.message = "Williamson pipeline succeeded.";
        return output;
    }

    output.message = "No non-planar biconnected component was found.";
    return output;
}

int originalEdgeIdForDart(
    const PreparedPalmTree& prepared,
    int dartId
) {
    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        return -1;
    }

    return prepared.darts[dartId].originalEdgeId;
}

int findSkeletonEdgeContainingOriginalEdge(
    const HomKernelSignature& signature,
    int originalEdgeId
) {
    if (originalEdgeId < 0) {
        return -1;
    }

    for (const HomKernelSkeletonEdge& skeletonEdge : signature.skeletonEdges) {
        for (int edgeId : skeletonEdge.originalEdgeIds) {
            if (edgeId == originalEdgeId) {
                return skeletonEdge.id;
            }
        }
    }

    return -1;
}

int findCanonicalEdgeForSkeletonEdge(
    const CanonicalHomKernelSignature& canonical,
    int skeletonEdgeId
) {
    if (skeletonEdgeId < 0) {
        return -1;
    }

    for (int canonicalEdgeId = 0;
         canonicalEdgeId < static_cast<int>(canonical.canonicalEdgeToSkeletonEdge.size());
         ++canonicalEdgeId) {
        if (canonical.canonicalEdgeToSkeletonEdge[canonicalEdgeId]
            == skeletonEdgeId) {
            return canonicalEdgeId;
        }
    }

    return -1;
}

int resolveRoleCanonicalEdge(
    const PreparedPalmTree& prepared,
    const HomKernelSignature& signature,
    const CanonicalHomKernelSignature& canonical,
    int roleDart
) {
    const int originalEdgeId =
        originalEdgeIdForDart(
            prepared,
            roleDart
        );

    const int skeletonEdgeId =
        findSkeletonEdgeContainingOriginalEdge(
            signature,
            originalEdgeId
        );

    return findCanonicalEdgeForSkeletonEdge(
        canonical,
        skeletonEdgeId
    );
}

std::string buildRoleAwareKey(
    const std::string& canonicalKey,
    int canonicalFEdge,
    int canonicalAEdge,
    int canonicalBEdge
) {
    std::ostringstream out;

    out << canonicalKey
        << "roles="
        << "F:" << canonicalFEdge
        << ",A:" << canonicalAEdge
        << ",B:" << canonicalBEdge
        << ",";

    return out.str();
}

const HomKernelSkeletonEdge* findSkeletonEdgeById(
    const HomKernelSignature& signature,
    int skeletonEdgeId
) {
    for (const HomKernelSkeletonEdge& edge : signature.skeletonEdges) {
        if (edge.id == skeletonEdgeId) {
            return &edge;
        }
    }

    return nullptr;
}

std::vector<int> expandSkeletonEdges(
    const HomKernelSignature& signature,
    const std::vector<int>& selectedSkeletonEdgeIds
) {
    std::vector<int> originalEdgeIds;

    for (int skeletonEdgeId : selectedSkeletonEdgeIds) {
        const HomKernelSkeletonEdge* edge =
            findSkeletonEdgeById(
                signature,
                skeletonEdgeId
            );

        if (edge == nullptr) {
            continue;
        }

        originalEdgeIds.insert(
            originalEdgeIds.end(),
            edge->originalEdgeIds.begin(),
            edge->originalEdgeIds.end()
        );
    }

    return originalEdgeIds;
}

bool verifySkeletonSubset(
    const PreparedPalmTree& prepared,
    const HomKernelSignature& signature,
    const std::vector<int>& selectedSkeletonEdgeIds,
    KuratowskiSubdivisionVerification& verification
) {
    std::vector<int> originalEdgeIds =
        expandSkeletonEdges(
            signature,
            selectedSkeletonEdgeIds
        );

    if (originalEdgeIds.empty()) {
        return false;
    }

    KuratowskiSubdivisionVerifier verifier;
    verification =
        verifier.verify(
            prepared,
            originalEdgeIds
        );

    return verification.valid;
}

bool chooseSubsetRecursive(
    const PreparedPalmTree& prepared,
    const HomKernelSignature& signature,
    int targetSize,
    int index,
    std::vector<int>& chosenSkeletonEdges,
    KuratowskiSubdivisionVerification& selectedVerification
) {
    if (static_cast<int>(chosenSkeletonEdges.size()) == targetSize) {
        return verifySkeletonSubset(
            prepared,
            signature,
            chosenSkeletonEdges,
            selectedVerification
        );
    }

    if (index >= static_cast<int>(signature.skeletonEdges.size())) {
        return false;
    }

    const int remaining =
        static_cast<int>(signature.skeletonEdges.size()) - index;

    const int needed =
        targetSize - static_cast<int>(chosenSkeletonEdges.size());

    if (remaining < needed) {
        return false;
    }

    const int skeletonEdgeId =
        signature.skeletonEdges[index].id;

    chosenSkeletonEdges.push_back(skeletonEdgeId);

    if (chooseSubsetRecursive(
            prepared,
            signature,
            targetSize,
            index + 1,
            chosenSkeletonEdges,
            selectedVerification
        )) {
        return true;
    }

    chosenSkeletonEdges.pop_back();

    return chooseSubsetRecursive(
        prepared,
        signature,
        targetSize,
        index + 1,
        chosenSkeletonEdges,
        selectedVerification
    );
}

bool findOfflineKuratowskiSelection(
    const PreparedPalmTree& prepared,
    const HomKernelSignature& signature,
    std::vector<int>& selectedSkeletonEdgeIds,
    KuratowskiSubdivisionVerification& verification
) {
    KuratowskiSubdivisionVerifier verifier;

    std::vector<int> allOriginalEdgeIds;

    for (const HomKernelSkeletonEdge& edge : signature.skeletonEdges) {
        allOriginalEdgeIds.insert(
            allOriginalEdgeIds.end(),
            edge.originalEdgeIds.begin(),
            edge.originalEdgeIds.end()
        );
    }

    verification =
        verifier.verify(
            prepared,
            allOriginalEdgeIds
        );

    if (verification.valid) {
        selectedSkeletonEdgeIds.clear();

        for (const HomKernelSkeletonEdge& edge : signature.skeletonEdges) {
            selectedSkeletonEdgeIds.push_back(edge.id);
        }

        return true;
    }

    for (int subsetSize = 9;
         subsetSize <= static_cast<int>(signature.skeletonEdges.size());
         ++subsetSize) {
        std::vector<int> chosen;

        if (chooseSubsetRecursive(
                prepared,
                signature,
                subsetSize,
                0,
                chosen,
                verification
            )) {
            selectedSkeletonEdgeIds = chosen;
            return true;
        }
    }

    return false;
}

std::vector<int> mapSkeletonSelectionToCanonicalSelection(
    const CanonicalHomKernelSignature& canonical,
    const std::vector<int>& selectedSkeletonEdgeIds
) {
    std::set<int> selectedSkeletonSet(
        selectedSkeletonEdgeIds.begin(),
        selectedSkeletonEdgeIds.end()
    );

    std::vector<int> selectedCanonicalEdgeIds;

    for (int canonicalEdgeId = 0;
         canonicalEdgeId < static_cast<int>(canonical.canonicalEdgeToSkeletonEdge.size());
         ++canonicalEdgeId) {
        const int skeletonEdgeId =
            canonical.canonicalEdgeToSkeletonEdge[canonicalEdgeId];

        if (selectedSkeletonSet.find(skeletonEdgeId)
            != selectedSkeletonSet.end()) {
            selectedCanonicalEdgeIds.push_back(canonicalEdgeId);
        }
    }

    return selectedCanonicalEdgeIds;
}

std::string vectorToInitializer(const std::vector<int>& values) {
    std::ostringstream out;

    out << "{";

    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
        if (i > 0) {
            out << ", ";
        }

        out << values[i];
    }

    out << "}";

    return out.str();
}

bool generateEntryForGraph(
    const NamedGraph& namedGraph,
    GeneratedEntry& entry,
    std::string& message
) {
    PipelineOutput pipeline =
        runWilliamsonPipeline(namedGraph.graph);

    if (!pipeline.valid) {
        message = pipeline.message;
        return false;
    }

    HomKernelSignatureBuilder signatureBuilder;
    HomKernelSignature signature =
        signatureBuilder.build(
            pipeline.prepared,
            pipeline.kernel.originalEdgeIds
        );

    if (!signature.valid) {
        message = signature.message;
        return false;
    }

    HomKernelCanonicalizer canonicalizer;
    CanonicalHomKernelSignature canonical =
        canonicalizer.canonicalize(signature);

    if (!canonical.valid) {
        message = canonical.message;
        return false;
    }

    std::vector<int> selectedSkeletonEdgeIds;
    KuratowskiSubdivisionVerification verification;

    if (!findOfflineKuratowskiSelection(
            pipeline.prepared,
            signature,
            selectedSkeletonEdgeIds,
            verification
        )) {
        message = "Offline selector could not find a Kuratowski subdivision.";
        return false;
    }

    const int canonicalFEdge =
        resolveRoleCanonicalEdge(
            pipeline.prepared,
            signature,
            canonical,
            pipeline.context.fDart
        );

    const int canonicalAEdge =
        resolveRoleCanonicalEdge(
            pipeline.prepared,
            signature,
            canonical,
            pipeline.context.aDart
        );

    const int canonicalBEdge =
        resolveRoleCanonicalEdge(
            pipeline.prepared,
            signature,
            canonical,
            pipeline.context.bDart
        );

    if (canonicalFEdge < 0
        || canonicalAEdge < 0
        || canonicalBEdge < 0) {
        std::ostringstream out;
        out << "Could not resolve F/A/B canonical role edges. "
            << "F=" << canonicalFEdge
            << ", A=" << canonicalAEdge
            << ", B=" << canonicalBEdge;

        message = out.str();
        return false;
    }

    std::vector<int> selectedCanonicalEdgeIds =
        mapSkeletonSelectionToCanonicalSelection(
            canonical,
            selectedSkeletonEdgeIds
        );

    entry.graphName = namedGraph.name;
    entry.canonicalKey = canonical.key;
    entry.canonicalFEdge = canonicalFEdge;
    entry.canonicalAEdge = canonicalAEdge;
    entry.canonicalBEdge = canonicalBEdge;
    entry.roleAwareKey =
        buildRoleAwareKey(
            canonical.key,
            canonicalFEdge,
            canonicalAEdge,
            canonicalBEdge
        );
    entry.selectedCanonicalEdgeIds = selectedCanonicalEdgeIds;
    entry.type = verification.type;

    message = "Generated role-aware entry.";
    return true;
}

std::vector<NamedGraph> seedGraphs() {
    return {
        {"K5", ht::test::buildK5()},
        {"K3,3", ht::test::buildK33()},
        {"Subdivided K5", ht::test::buildSubdividedK5()},
        {"Subdivided K3,3", ht::test::buildSubdividedK33()},
        {"Partially subdivided K5", ht::test::buildPartiallySubdividedK5()},
        {"Partially subdivided K3,3", ht::test::buildPartiallySubdividedK33()},
        {"Petersen graph", ht::test::buildPetersenGraph()},
        {"DM Rijeseni 14b", ht::test::buildDMRijeseni14b()},
        {"DM Rijeseni 15", ht::test::buildDMRijeseni15()},
        {"DM zsr 10", ht::test::buildDMzsr10()}
    };
}

void writeGeneratedTable(
    std::ostream& out,
    const std::map<std::string, GeneratedEntry>& entriesByKey
) {
    out << "// Generated role-aware HOMKERNEL lookup table entries.\n";
    out << "// This file is produced by tools/HomKernelTableGenerator.cpp.\n";
    out << "// Do not edit manually.\n\n";

    out << "static const std::vector<HomKernelTableEntry> table = {\n";

    bool first = true;

    for (const auto& [key, entry] : entriesByKey) {
        if (!first) {
            out << ",\n";
        }

        first = false;

        out << "    {\n";
        out << "        // source: " << entry.graphName
            << ", type: " << certificateTypeToString(entry.type)
            << "\n";
        out << "        // canonical roles: F="
            << entry.canonicalFEdge
            << ", A=" << entry.canonicalAEdge
            << ", B=" << entry.canonicalBEdge
            << "\n";
        out << "        \"";
        out << entry.roleAwareKey;
        out << "\",\n";
        out << "        "
            << vectorToInitializer(entry.selectedCanonicalEdgeIds)
            << "\n";
        out << "    }";
    }

    out << "\n};\n";
}

void writeReport(
    std::ostream& out,
    const std::map<std::string, GeneratedEntry>& entriesByKey,
    const std::vector<std::string>& messages
) {
    out << "Role-aware HOMKERNEL Table Generator Report\n";
    out << "========================================\n\n";

    out << "Generated unique role-aware entries: "
        << entriesByKey.size()
        << "\n\n";

    for (const auto& [key, entry] : entriesByKey) {
        out << "----------------------------------------\n";
        out << "Source graph: " << entry.graphName << "\n";
        out << "Type: " << certificateTypeToString(entry.type) << "\n";

        out << "Canonical key:\n";
        out << entry.canonicalKey << "\n";

        out << "Canonical role edges:\n";
        out << "  F = " << entry.canonicalFEdge << "\n";
        out << "  A = " << entry.canonicalAEdge << "\n";
        out << "  B = " << entry.canonicalBEdge << "\n";

        out << "Role-aware key:\n";
        out << entry.roleAwareKey << "\n";

        out << "Selected canonical edges: ";

        for (int edgeId : entry.selectedCanonicalEdgeIds) {
            out << edgeId << " ";
        }

        out << "\n\n";
    }

    if (!messages.empty()) {
        out << "Messages:\n";

        for (const std::string& message : messages) {
            out << "  " << message << "\n";
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    std::string tablePath = "generated_homkernel_table.inc";
    std::string reportPath = "homkernel_table_generator_report.txt";

    if (argc >= 2) {
        tablePath = argv[1];
    }

    if (argc >= 3) {
        reportPath = argv[2];
    }

    std::map<std::string, GeneratedEntry> entriesByKey;
    std::vector<std::string> messages;

    for (const NamedGraph& graph : seedGraphs()) {
        GeneratedEntry entry;
        std::string message;

        const bool generated =
            generateEntryForGraph(
                graph,
                entry,
                message
            );

        if (!generated) {
            messages.push_back(
                graph.name + ": " + message
            );
            continue;
        }

        auto existing = entriesByKey.find(entry.roleAwareKey);

        if (existing == entriesByKey.end()) {
            entriesByKey[entry.roleAwareKey] = entry;
            continue;
        }

        if (existing->second.selectedCanonicalEdgeIds
            != entry.selectedCanonicalEdgeIds) {
            messages.push_back(
                "Conflict for role-aware key generated by "
                + graph.name
            );
        }
    }

    std::ofstream tableOutput(tablePath);

    if (!tableOutput) {
        std::cerr << "Could not open table output file: "
                  << tablePath
                  << "\n";
        return 1;
    }

    writeGeneratedTable(
        tableOutput,
        entriesByKey
    );

    std::ofstream reportOutput(reportPath);

    if (!reportOutput) {
        std::cerr << "Could not open report output file: "
                  << reportPath
                  << "\n";
        return 1;
    }

    writeReport(
        reportOutput,
        entriesByKey,
        messages
    );

    std::cout << "Generated role-aware HOMKERNEL table: "
              << tablePath
              << "\n";

    std::cout << "Generated report: "
              << reportPath
              << "\n";

    return 0;
}