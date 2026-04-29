# Embedding Phase Notes

The embedding phase should construct a combinatorial embedding, not a geometric drawing.

A combinatorial embedding means:

- for each vertex `v`, output a cyclic order of incident darts/edges around `v`;
- this rotation system determines the topological embedding up to the outer face.

In this repository, the embedding is represented by:

```cpp
std::vector<std::vector<int>> rotationDarts;
std::vector<std::vector<int>> rotationNeighbors;
```

The validator checks:

- every dart appears exactly once in the rotation of its source vertex;
- every reverse dart relation is symmetric;
- face traversal gives Euler formula `V - E + F = 2` for connected planar components.
