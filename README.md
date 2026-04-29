# HT Planarity C++

Clean restart repository for a C++ implementation of the Hopcroft-Tarjan planarity pipeline.

Target scope:

1. Planarity decision in linear time.
2. Combinatorial embedding when the graph is planar.
3. Kuratowski certificate when the graph is non-planar.

Current starter state:

- Graph model with stable edge IDs.
- Biconnected component decomposition.
- Component preprocessing: local graph, DFS numbering, lowpt1/lowpt2, HT phi ordering.
- Prepared palm tree / dart model.
- Imported/refactored strong planarity and embedding prototypes.
- Embedding validator using Euler face check.
- Certificate module placeholders.

Important:

The certificate module is intentionally a placeholder in this starter. Do not claim final Kuratowski extraction correctness until the Williamson/segment-forest extraction is implemented and tested.
