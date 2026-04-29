#pragma once

#include <string>
#include <vector>

namespace ht {

enum class KuratowskiType {
    Unknown,
    K5Subdivision,
    K33Subdivision
};

struct KuratowskiCertificate {
    KuratowskiType type = KuratowskiType::Unknown;

    // Original input edge IDs that form the certificate subgraph.
    std::vector<int> originalEdgeIds;

    std::string message;

    bool empty() const {
        return originalEdgeIds.empty();
    }
};

} // namespace ht
