#pragma once

#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonFList.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

struct WilliamsonLinkWitness {
    bool exists = false;

    int sourceNode = -1;
    int spanNode = -1;

    int backDart = -1;
    int backTailVertex = -1;
    int headVertex = -1;
    int headDfs = -1;
};

class WilliamsonLinkOracle {
public:
    WilliamsonLinkOracle(
        const PreparedPalmTree& prepared,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList
    );

    WilliamsonLinkWitness findLinkToOpenSpan(
        int sourceNode,
        int spanNode
    ) const;

    bool linksToOpenSpan(
        int sourceNode,
        int spanNode
    ) const;

private:
    const PreparedPalmTree& prepared_;
    const SegmentMetadataTable& metadata_;
    const WilliamsonFList& fList_;

    const SegmentMetadata& metadataForNode(int nodeId) const;
};

} // namespace ht