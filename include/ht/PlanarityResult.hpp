#pragma once

#include <string>

#include "ht/certificate/KuratowskiCertificate.hpp"
#include "ht/embedding/Embedding.hpp"

namespace ht {

struct PlanarityResult {
    bool planar = false;

    // Valid when planar == true and an embedding was actually constructed.
    Embedding embedding;

    // Valid when planar == false and certificate extraction is implemented.
    KuratowskiCertificate certificate;

    std::string message;
};

} // namespace ht
