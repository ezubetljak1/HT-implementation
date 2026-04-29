#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "ht/PlanarityTester.hpp"
#include "ht/certificate/KuratowskiCertificate.hpp"

#include "TestGraphs.hpp"

using namespace ht;

namespace {

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

void printGraphEdges(std::ostream& out, const Graph& graph) {
    out << "Original graph edges:\n";

    for (const Edge& edge : graph.edges()) {
        out << "  edge id=" << edge.id
            << ", originalId=" << edge.originalId
            << ": " << edge.u << " -- " << edge.v
            << "\n";
    }
}

void printEmbedding(
    std::ostream& out,
    const Embedding& embedding,
    int vertexCount
) {
    out << "Embedding rotation by original vertices:\n";

    for (int v = 0; v < vertexCount; ++v) {
        out << "  vertex " << v << ":\n";

        if (v >= static_cast<int>(embedding.rotationOriginalNeighbors.size())
            || embedding.rotationOriginalNeighbors[v].empty()) {
            out << "    [empty rotation]\n";
            continue;
        }

        const std::vector<int>& neighbors =
            embedding.rotationOriginalNeighbors[v];

        const std::vector<int>& edgeIds =
            embedding.rotationOriginalEdgeIds[v];

        for (int i = 0; i < static_cast<int>(neighbors.size()); ++i) {
            out << "    neighbor=" << neighbors[i];

            if (i < static_cast<int>(edgeIds.size())) {
                out << ", originalEdgeId=" << edgeIds[i];
            }

            out << "\n";
        }
    }
}

void printCertificate(
    std::ostream& out,
    const KuratowskiCertificate& certificate
) {
    out << "Kuratowski certificate:\n";
    out << "  type = "
        << certificateTypeToString(certificate.type)
        << "\n";

    out << "  originalEdgeIds [size="
        << certificate.originalEdgeIds.size()
        << "]: ";

    for (int edgeId : certificate.originalEdgeIds) {
        out << edgeId << " ";
    }

    out << "\n";

    out << "  message:\n";
    out << "    " << certificate.message << "\n";
}

void inspectGraph(
    std::ostream& out,
    const std::string& name,
    const Graph& graph,
    bool buildEmbedding = true
) {
    out << "========================================\n";
    out << "Graph: " << name << "\n";
    out << "V = " << graph.vertexCount()
        << ", E = " << graph.edgeCount()
        << "\n\n";

    printGraphEdges(out, graph);

    out << "\nRunning planarity tester...\n";

    PlanarityTester tester;
    PlanarityResult result = tester.test(graph, buildEmbedding);

    out << "Planar = "
        << (result.planar ? "true" : "false")
        << "\n";

    out << "Message:\n";
    out << "  " << result.message << "\n\n";

    if (result.planar) {
        if (buildEmbedding) {
            printEmbedding(out, result.embedding, graph.vertexCount());
        } else {
            out << "Embedding was not requested.\n";
        }
    } else {
        printCertificate(out, result.certificate);
    }

    out << "\n";
}

void writeReport(std::ostream& out) {
    out << "HT Planarity Inspection Report\n";
    out << "========================================\n\n";

    inspectGraph(out, "Empty graph", ht::test::buildEmptyGraph());
    inspectGraph(out, "Single vertex", ht::test::buildSingleVertex());
    inspectGraph(out, "Single edge", ht::test::buildSingleEdge());

    inspectGraph(out, "Path graph", ht::test::buildPathGraph());
    inspectGraph(out, "Cycle graph", ht::test::buildCycleGraph());
    inspectGraph(out, "Triangle", ht::test::buildTriangle());
    inspectGraph(out, "K4", ht::test::buildK4());
    inspectGraph(out, "Wheel graph 5", ht::test::buildWheelGraph5());

    inspectGraph(
        out,
        "Two triangles connected by bridge",
        ht::test::buildTwoTrianglesConnectedByBridge()
    );

    inspectGraph(out, "Star graph 5", ht::test::buildStarGraph5());
    inspectGraph(out, "Butterfly graph", ht::test::buildButterflyGraph());
    inspectGraph(out, "Figure eight graph", ht::test::buildFigureEightGraph());
    inspectGraph(out, "Ladder graph 3", ht::test::buildLadderGraph3());
    inspectGraph(out, "Triangular prism", ht::test::buildTriangularPrism());
    inspectGraph(out, "Cube graph", ht::test::buildCubeGraph());

    inspectGraph(
        out,
        "Disconnected planar graph",
        ht::test::buildDisconnectedPlanarGraph()
    );

    inspectGraph(out, "K5 minus one edge", ht::test::buildK5MinusOneEdge());
    inspectGraph(out, "K3,3 minus one edge", ht::test::buildK33MinusOneEdge());

    inspectGraph(out, "K5", ht::test::buildK5());
    inspectGraph(out, "K3,3", ht::test::buildK33());
    inspectGraph(out, "Subdivided K5", ht::test::buildSubdividedK5());
    inspectGraph(out, "Subdivided K3,3", ht::test::buildSubdividedK33());

    inspectGraph(
        out,
        "Partially subdivided K5",
        ht::test::buildPartiallySubdividedK5()
    );

    inspectGraph(
        out,
        "Partially subdivided K3,3",
        ht::test::buildPartiallySubdividedK33()
    );

    inspectGraph(
        out,
        "Disconnected non-planar graph",
        ht::test::buildDisconnectedNonPlanarGraph()
    );

    inspectGraph(out, "Petersen graph", ht::test::buildPetersenGraph());
    inspectGraph(out, "Octahedral graph", ht::test::buildOctahedralGraph());

    inspectGraph(out, "DM Rijeseni 14a", ht::test::buildDMRijeseni14a());
    inspectGraph(out, "DM Rijeseni 14b", ht::test::buildDMRijeseni14b());
    inspectGraph(out, "DM Rijeseni 15", ht::test::buildDMRijeseni15());
    inspectGraph(out, "DM zsr 10", ht::test::buildDMzsr10());
    inspectGraph(out, "DM zsr 14", ht::test::buildDMzsr14());
}

} // namespace

int main(int argc, char** argv) {
    std::string outputPath = "planarity_inspection_report.txt";

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

    std::cout << "Inspection report written to: "
              << outputPath
              << "\n";

    return 0;
}