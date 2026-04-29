#include "ht/certificate/HomKernelLookupTable.hpp"

namespace ht {
namespace {

struct HomKernelTableEntry {
    const char* shapeKey;
    std::vector<int> selectedSkeletonEdgeIds;
};

const std::vector<HomKernelTableEntry>& entries() {
    static const std::vector<HomKernelTableEntry> table = {
        {
            // K5
            "V=5;E=10;deg=4,4,4,4,4,;edges=0-1[1],0-4[1],0-3[1],0-2[1],1-2[1],1-3[1],1-4[1],2-4[1],2-3[1],3-4[1],",
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
        },
        {
            // Subdivided K5
            "V=5;E=10;deg=4,4,4,4,4,;edges=0-1[2],0-4[2],0-3[2],0-2[2],1-2[2],1-3[2],1-4[2],2-4[2],2-3[2],3-4[2],",
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
        },
        {
            // Partially subdivided K5
            "V=5;E=10;deg=4,4,4,4,4,;edges=0-1[1],0-4[1],0-3[1],0-2[1],1-2[1],1-3[1],1-4[1],2-4[1],2-3[2],3-4[1],",
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
        },
        {
            // K3,3
            "V=6;E=9;deg=3,3,3,3,3,3,;edges=0-1[1],0-5[1],0-4[1],1-2[1],1-3[1],2-5[1],2-4[1],3-4[1],3-5[1],",
            {0, 1, 2, 3, 4, 5, 6, 7, 8}
        },
        {
            // Subdivided K3,3
            "V=6;E=9;deg=3,3,3,3,3,3,;edges=0-1[2],0-5[2],0-4[2],1-2[2],1-3[2],2-5[2],2-4[2],3-4[2],3-5[2],",
            {0, 1, 2, 3, 4, 5, 6, 7, 8}
        },
        {
            // Partially subdivided K3,3
            "V=6;E=9;deg=3,3,3,3,3,3,;edges=0-1[1],0-5[1],0-4[1],1-2[1],1-3[1],2-5[1],2-4[2],3-4[1],3-5[1],",
            {0, 1, 2, 3, 4, 5, 6, 7, 8}
        },
        {
            // DM Rijeseni 14b, reduced to K3,3 subdivision.
            "V=6;E=9;deg=3,3,3,3,3,3,;edges=0-1[1],0-4[2],0-5[1],1-2[1],1-3[1],2-5[1],2-4[1],3-5[1],3-4[1],",
            {0, 1, 2, 3, 4, 5, 6, 7, 8}
        },
        {
            // Petersen HOMKERNEL form.
            // Select all except skeleton edge 11.
            "V=8;E=12;deg=3,3,3,3,3,3,3,3,;edges=0-2[2],0-3[1],0-5[1],1-2[1],1-7[1],1-5[1],2-4[1],3-4[1],3-7[1],4-6[1],5-6[1],6-7[2],",
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
        },
        {
            // DM Rijeseni 15 HOMKERNEL form.
            // Select all except skeleton edge 1.
            "V=7;E=11;deg=4,3,3,3,3,3,3,;edges=0-1[1],0-3[1],0-6[1],0-4[1],1-2[1],1-3[1],2-4[1],2-6[1],3-5[1],4-5[1],5-6[1],",
            {0, 2, 3, 4, 5, 6, 7, 8, 9, 10}
        },
        {
            // DM zsr 10 HOMKERNEL form.
            "V=7;E=13;deg=3,4,5,3,3,5,3,;edges=0-1[2],0-5[1],0-4[1],1-6[2],1-2[1],1-3[1],2-5[2],2-5[1],2-6[1],2-4[1],3-4[1],3-5[1],5-6[1],",
            {0, 1, 2, 4, 5, 6, 9, 10, 11}
        }
    };

    return table;
}

} // namespace

HomKernelLookupResult HomKernelLookupTable::lookup(
    const std::string& shapeKey
) const {
    HomKernelLookupResult result;

    for (const HomKernelTableEntry& entry : entries()) {
        if (shapeKey == entry.shapeKey) {
            result.found = true;
            result.selectedSkeletonEdgeIds = entry.selectedSkeletonEdgeIds;
            result.message = "HOMKERNEL runtime table entry found.";
            return result;
        }
    }

    result.message =
        "No HOMKERNEL runtime table entry exists for this shape key.";

    return result;
}

} // namespace ht