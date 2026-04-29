# Kuratowski Certificate Plan

The current starter does not yet implement Kuratowski extraction.

Planned direction:

1. During strong-planarity, preserve the exact failure witness.
2. Represent cycle/path/segment structure explicitly.
3. Build a segment forest or equivalent structure.
4. When a forbidden interlacing is found, trace original edge IDs forming a subdivision.
5. Return:

```cpp
struct KuratowskiCertificate {
    KuratowskiType type;              // Unknown, K5Subdivision, K33Subdivision
    std::vector<int> originalEdgeIds; // edges in the original input graph
};
```

Important:

Do not return only local edge IDs. The final certificate must be expressed in original graph edge IDs.
