#pragma once

#include <iosfwd>

#include "ht/preprocess/PreparedPalmTree.hpp"
#include "ht/preprocess/ComponentPreprocessor.hpp"
#include "ht/strong/StrongPlanarityTester.hpp"

namespace ht {

class DebugPrinter {
public:
    static void printPreprocessedComponent(
        const PreprocessedComponent& component,
        std::ostream& out
    );

    static void printPreparedPalmTree(
        const PreparedPalmTree& prepared,
        std::ostream& out
    );

    static void printStrongPlanarityFailure(
        const PreparedPalmTree& prepared,
        const StrongPlanarityFailure& failure,
        std::ostream& out
    );
};

} // namespace ht
