# Linear-Time Invariants

To honestly claim linear time, keep these rules:

1. No `std::sort` in the core HT ordering phase.
   - HT ordering must use bucket sort because phi values are in a bounded range.

2. Every edge and dart has a stable integer ID.
   - Do not search for reverse darts by scanning adjacency lists.
   - `Dart::rev` must be filled once and used directly.

3. Every DFS edge/back edge needed later has a witness.
   - `lowpt1` and `lowpt2` should not only store numbers.
   - They should also keep enough witness information for certificate extraction.

4. Do not rebuild large subgraphs repeatedly.
   - Certificate extraction must reuse path/segment metadata.

5. Use adjacency lists, not matrices.
