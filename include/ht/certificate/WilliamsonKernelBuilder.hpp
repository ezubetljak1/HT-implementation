#pragma once

#include "ht/certificate/PathTree.hpp"
#include "ht/certificate/SegmentMetadata.hpp"
#include "ht/certificate/WilliamsonContext.hpp"
#include "ht/certificate/WilliamsonFList.hpp"
#include "ht/certificate/WilliamsonKernel.hpp"
#include "ht/certificate/WilliamsonSegfoPath.hpp"
#include "ht/preprocess/PreparedPalmTree.hpp"

namespace ht {

class WilliamsonKernelBuilder {
public:
    WilliamsonKernel buildKernelFromSegfoPath(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonFList& fList,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    ) const;

    WilliamsonKernel buildKernelFromSegfoPath(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const SegmentMetadataTable& metadata,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    ) const;

    WilliamsonKernel buildKernelFromSegfoPath(
        const PreparedPalmTree& prepared,
        const PathTree& pathTree,
        const WilliamsonContext& context,
        const WilliamsonSegfoPath& segfoPath
    ) const;
};

} // namespace ht