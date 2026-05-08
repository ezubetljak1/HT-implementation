#include "ht/certificate/WilliamsonKernelBuilder.hpp"

#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonPathKernelBuilder.hpp"

namespace ht {

WilliamsonKernel WilliamsonKernelBuilder::buildKernelFromSegfoPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonPathKernelBuilder pathKernelBuilder;

    return pathKernelBuilder.buildBasicCase1(
        prepared,
        pathTree,
        metadata,
        context,
        segfoPath
    );
}

WilliamsonKernel WilliamsonKernelBuilder::buildKernelFromSegfoPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    const SegmentMetadataTable metadata =
        SegmentMetadataBuilder().build(
            prepared,
            pathTree
        );

    return buildKernelFromSegfoPath(
        prepared,
        pathTree,
        metadata,
        context,
        segfoPath
    );
}

} // namespace ht