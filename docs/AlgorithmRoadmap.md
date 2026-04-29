# Algorithm Roadmap

## Phase 1: Input and decomposition

- Build a simple undirected graph.
- Every original edge receives a stable `originalEdgeId`.
- Split graph into biconnected components.

## Phase 2: Hopcroft-Tarjan preprocessing

For each biconnected component:

- Build a local graph.
- Run DFS.
- Compute DFS numbers, parents, children.
- Classify active HT arcs:
  - tree arcs: parent -> child
  - fronds/back arcs: descendant -> ancestor
- Compute `lowpt1`, `lowpt2`.
- Compute HT `phi`/weight.
- Bucket-sort active outgoing arcs by `phi`.

## Phase 3: Strong planarity

Use the HT strong-planarity phase over the prepared palm tree.

Current starter has a refactored prototype, but it still needs deeper validation against the paper.

## Phase 4: Embedding

If strong-planarity succeeds, use the alpha side assignments to construct a combinatorial embedding.

Output format:

- `rotationDarts[v]`: cyclic order of directed darts leaving vertex `v`.
- `rotationNeighbors[v]`: same order translated into neighbor IDs.

## Phase 5: Kuratowski certificate

If strong-planarity fails, extract a subdivision of `K5` or `K3,3`.

This module is not implemented yet. It should be based on the segment/path-tree failure witness structure, not on the Left-Right conflict stack.
