#include "ht/certificate/WilliamsonKernelBuilder.hpp"

#include "ht/certificate/SegmentMetadataBuilder.hpp"
#include "ht/certificate/WilliamsonFListBuilder.hpp"
#include "ht/certificate/WilliamsonPathKernelBuilder.hpp"
#include "ht/certificate/WilliamsonSegmentList.hpp"

#include <vector>

namespace ht {

namespace {

void addUniqueNode(
    std::vector<int>& nodes,
    int nodeId
) {
    if (nodeId < 0) {
        return;
    }

    for (int existing : nodes) {
        if (existing == nodeId) {
            return;
        }
    }

    nodes.push_back(nodeId);
}

} // namespace

WilliamsonKernel WilliamsonKernelBuilder::buildKernelFromSegfoPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonFList& fList,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonPathKernelBuilder pathKernelBuilder;

    return pathKernelBuilder.build(
        prepared,
        pathTree,
        metadata,
        fList,
        context,
        segfoPath
    );
}

WilliamsonKernel WilliamsonKernelBuilder::buildKernelFromSegfoPath(
    const PreparedPalmTree& prepared,
    const PathTree& pathTree,
    const SegmentMetadataTable& metadata,
    const WilliamsonContext& context,
    const WilliamsonSegfoPath& segfoPath
) const {
    WilliamsonSegmentList segmentList;
    segmentList.valid = true;
    segmentList.message =
        "Compatibility SEGLIST built from SEGFO path and F node.";

    for (int nodeId : segfoPath.segmentPathNodes) {
        addUniqueNode(segmentList.segmentNodes, nodeId);
    }

    addUniqueNode(segmentList.segmentNodes, context.fNode);

    WilliamsonFListBuilder fListBuilder;

    WilliamsonFList fList =
        fListBuilder.buildFromSegmentList(
            prepared,
            pathTree,
            metadata,
            segmentList,
            context.fNode
        );

    if (!fList.valid) {
        WilliamsonKernel kernel;
        kernel.context = context;
        kernel.segfoPath = segfoPath;
        kernel.valid = false;
        kernel.message =
            "Compatibility overload failed to build FLIST from SEGFO path.";
        return kernel;
    }

    return buildKernelFromSegfoPath(
        prepared,
        pathTree,
        metadata,
        fList,
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