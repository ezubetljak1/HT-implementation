#include "ht/debug/DebugPrinter.hpp"

#include <ostream>

namespace ht {

void DebugPrinter::printPreprocessedComponent(
    const PreprocessedComponent& component,
    std::ostream& out
) {
    out << "PreprocessedComponent\n";
    out << "n = " << component.context.localGraph.vertexCount() << "\n";

    for (int v = 0; v < component.context.localGraph.vertexCount(); ++v) {
        out << "v=" << v
            << " original=" << component.context.localToOriginal[v]
            << " number=" << component.number[v]
            << " parent=" << component.parent[v]
            << " lowpt1=" << component.lowpt1[v]
            << " lowpt2=" << component.lowpt2[v]
            << "\n";
    }
}

void DebugPrinter::printPreparedPalmTree(
    const PreparedPalmTree& prepared,
    std::ostream& out
) {
    out << "PreparedPalmTree\n";
    out << "n = " << prepared.n << "\n";
    out << "edgeCount = " << prepared.edgeCount << "\n";
    out << "rootTreeDart = " << prepared.rootTreeDart << "\n";

    for (const Dart& d : prepared.darts) {
        out << "dart " << d.id
            << ": " << d.from << " -> " << d.to
            << " rev=" << d.rev
            << " edgeId=" << d.edgeId
            << " originalEdgeId=" << d.originalEdgeId
            << " tree=" << d.isTree
            << " back=" << d.isBack
            << " phi=" << d.phi
            << "\n";
    }
}

} // namespace ht
