#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

#include "ht/Graph.hpp"
#include "ht/bcc/BiconnectedComponents.hpp"
#include "ht/debug/DebugPrinter.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/preprocess/PreparedPalmTreeBuilder.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"
#include "ht/PlanarityTester.hpp"
#include "ht/certificate/KuratowskiExtractor.hpp"

namespace {

ht::Graph buildK33() {
    ht::Graph g(6);

    for (int u = 0; u < 3; ++u) {
        for (int v = 3; v < 6; ++v) {
            g.addEdge(u, v);
        }
    }

    return g;
}

ht::Graph buildSubdividedK5() {
    ht::Graph g(15);

    int subdivisionVertex = 5;

    for (int u = 0; u < 5; ++u) {
        for (int v = u + 1; v < 5; ++v) {
            int s = subdivisionVertex++;

            g.addEdge(u, s);
            g.addEdge(s, v);
        }
    }

    return g;
}

void inspectCandidateSubgraph(
    const ht::Graph& originalGraph,
    const ht::KuratowskiCertificate& certificate
) {
    std::cout << "\nCandidate original edge IDs [size="
              << certificate.originalEdgeIds.size()
              << "]: ";

    for (int edgeId : certificate.originalEdgeIds) {
        std::cout << edgeId << " ";
    }

    std::cout << "\n";

    std::unordered_set<int> selected;
    for (int edgeId : certificate.originalEdgeIds) {
        selected.insert(edgeId);
    }

    std::cout << "Original edges not in candidate: ";
    for (const ht::Edge& edge : originalGraph.edges()) {
        if (selected.find(edge.originalId) == selected.end()) {
            std::cout << edge.originalId
                      << "(" << edge.u << "-" << edge.v << ") ";
        }
    }
    std::cout << "\n";

    ht::Graph candidate(originalGraph.vertexCount());

    for (const ht::Edge& edge : originalGraph.edges()) {
        if (selected.find(edge.originalId) != selected.end()) {
            candidate.addEdgeWithOriginalId(edge.u, edge.v, edge.originalId);
        }
    }

    ht::PlanarityTester tester;
    ht::PlanarityResult result = tester.test(candidate, false);

    std::cout << "Candidate subgraph planarity = "
              << (result.planar ? "planar" : "non-planar")
              << "\n";
}


void inspectGraph(const std::string& name, const ht::Graph& graph) {
    std::cout << "========================================\n";
    std::cout << "Inspecting graph: " << name << "\n";
    std::cout << "V = " << graph.vertexCount()
              << ", E = " << graph.edgeCount()
              << "\n";

    ht::BiconnectedComponentsFinder finder;
    ht::Components components = finder.find(graph);

    std::cout << "Biconnected components = " << components.size() << "\n";

    ht::ComponentPreprocessor preprocessor;
    ht::PreparedPalmTreeBuilder builder;

    for (int i = 0; i < static_cast<int>(components.size()); ++i) {
        const ht::Component& component = components[i];

        if (component.size() <= 1) {
            std::cout << "Component " << i
                      << " skipped because size <= 1.\n";
            continue;
        }

        std::cout << "\nComponent " << i
                  << ", edges = " << component.size()
                  << "\n";

        ht::PreprocessedComponent preprocessed =
            preprocessor.preprocess(component);

        ht::PreparedPalmTree prepared =
            builder.build(preprocessed);

        if (prepared.rootTreeDart == -1) {
            std::cout << "No rootTreeDart for this component.\n";
            continue;
        }

        ht::StrongPlanarityTester tester(prepared, prepared.number);
        std::vector<ht::Side> alpha;

        bool planar = tester.run(prepared.rootTreeDart, alpha);

        std::cout << "Strong-planarity result = "
                  << (planar ? "planar" : "non-planar")
                  << "\n";

        if (!planar) {
            const ht::StrongPlanarityFailure& failure = tester.failure();

            std::cout << "\n";
            ht::DebugPrinter::printStrongPlanarityFailure(
                prepared,
                failure,
                std::cout
            );

            ht::KuratowskiExtractor extractor;
            ht::KuratowskiCertificate certificate =
                extractor.extractFromFailure(prepared, failure);

            inspectCandidateSubgraph(graph, certificate);

            return;
        }
    }

    std::cout << "No strong-planarity failure found.\n";
}

} // namespace

int main() {
    inspectGraph("K3,3", buildK33());
    inspectGraph("Subdivided K5", buildSubdividedK5());

    return 0;
}