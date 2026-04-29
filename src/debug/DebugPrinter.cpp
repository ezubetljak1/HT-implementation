#include "ht/debug/DebugPrinter.hpp"

#include <ostream>
#include <string>
#include <vector>

namespace {

std::string failureTypeToString(ht::StrongPlanarityFailureType type) {
    switch (type) {
        case ht::StrongPlanarityFailureType::None:
            return "None";
        case ht::StrongPlanarityFailureType::UnresolvableLeftInterlace:
            return "UnresolvableLeftInterlace";
        case ht::StrongPlanarityFailureType::BothSidesAttachAboveW0:
            return "BothSidesAttachAboveW0";
    }

    return "Unknown";
}

void printIntVector(
    const std::string& label,
    const std::vector<int>& values,
    std::ostream& out
) {
    out << label << " [size=" << values.size() << "]: ";

    for (int value : values) {
        out << value << " ";
    }

    out << "\n";
}

void printDartVector(
    const std::string& label,
    const ht::PreparedPalmTree& prepared,
    const std::vector<int>& dartIds,
    std::ostream& out
) {
    out << label << " [size=" << dartIds.size() << "]\n";

    for (int dartId : dartIds) {
        if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
            out << "  dart " << dartId << " INVALID\n";
            continue;
        }

        const ht::Dart& dart = prepared.darts[dartId];

        out << "  dart " << dart.id
            << ": " << dart.from << " -> " << dart.to
            << ", rev=" << dart.rev
            << ", edgeId=" << dart.edgeId
            << ", originalEdgeId=" << dart.originalEdgeId
            << ", tree=" << dart.isTree
            << ", back=" << dart.isBack
            << ", phi=" << dart.phi
            << "\n";
    }
}

void printSingleDart(
    const std::string& label,
    const ht::PreparedPalmTree& prepared,
    int dartId,
    std::ostream& out
) {
    out << label << ": ";

    if (dartId < 0 || dartId >= static_cast<int>(prepared.darts.size())) {
        out << dartId << " INVALID\n";
        return;
    }

    const ht::Dart& dart = prepared.darts[dartId];

    out << "dart " << dart.id
        << " (" << dart.from << " -> " << dart.to << ")"
        << ", originalEdgeId=" << dart.originalEdgeId
        << ", tree=" << dart.isTree
        << ", back=" << dart.isBack
        << ", phi=" << dart.phi
        << "\n";
}

} // namespace

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

void DebugPrinter::printStrongPlanarityFailure(
    const PreparedPalmTree& prepared,
    const StrongPlanarityFailure& failure,
    std::ostream& out
) {
    out << "StrongPlanarityFailure\n";
    out << "type = " << failureTypeToString(failure.type) << "\n";
    out << "hasFailure = " << failure.hasFailure() << "\n";
    out << "message = " << failure.message << "\n";

    out << "\nCycle context\n";
    out << "x = " << failure.x << "\n";
    out << "y = " << failure.y << "\n";
    out << "w0 = " << failure.w0 << "\n";
    out << "wk = " << failure.wk << "\n";

    out << "\nImportant darts\n";
    printSingleDart("rootTreeDart", prepared, failure.rootTreeDart, out);
    printSingleDart("cycleRootDart", prepared, failure.cycleRootDart, out);
    printSingleDart("currentDart", prepared, failure.currentDart, out);
    printSingleDart("closingBackDart", prepared, failure.closingBackDart, out);

    out << "\nCycle darts\n";
    printDartVector("cycleStemDarts", prepared, failure.cycleStemDarts, out);
    printDartVector("cycleSpineDarts", prepared, failure.cycleSpineDarts, out);
    printDartVector("cycleTreeDarts", prepared, failure.cycleTreeDarts, out);

    out << "\nBlock witness\n";
    printIntVector("blockLeftAttachments", failure.blockLeftAttachments, out);
    printIntVector("blockRightAttachments", failure.blockRightAttachments, out);
    printDartVector("blockLeftSegments", prepared, failure.blockLeftSegments, out);
    printDartVector("blockRightSegments", prepared, failure.blockRightSegments, out);

    out << "\nStack top witness\n";
    printIntVector("stackTopLeftAttachments", failure.stackTopLeftAttachments, out);
    printIntVector("stackTopRightAttachments", failure.stackTopRightAttachments, out);
    printDartVector("stackTopLeftSegments", prepared, failure.stackTopLeftSegments, out);
    printDartVector("stackTopRightSegments", prepared, failure.stackTopRightSegments, out);
}

} // namespace ht
