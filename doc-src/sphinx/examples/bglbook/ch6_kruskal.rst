.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 6.1: Kruskal's Minimum Spanning Tree Algorithm
============================================

Overview
--------

This example demonstrates Kruskal's algorithm for finding a minimum spanning
tree (MST) of an undirected weighted graph. The MST connects all vertices
with the minimum total edge weight while avoiding cycles.

**Applications:**

- Network design (minimizing cable length to connect all nodes)
- Cluster analysis (finding natural groupings)
- Approximation algorithms for TSP and Steiner tree

Algorithm Description
--------------------

Kruskal's algorithm uses a greedy approach: sort edges by weight and add
them to the MST if they don't create a cycle. A union-find data structure
efficiently tracks which vertices are connected.

**Algorithm steps:**

1. Sort all edges by weight (ascending)
2. Initialize each vertex as its own component (union-find)
3. For each edge (u, v, weight) in sorted order:
   - If u and v are in different components:
     - Add edge to MST
     - Union the components containing u and v
4. Stop when MST has |V|-1 edges

**Time complexity**: O(E log E) for sorting + O(E α(V)) for union-find
≈ O(E log E) where α is the inverse Ackermann function (nearly constant)

NWGraph Implementation
---------------------

NWGraph provides a built-in Kruskal implementation that uses union-find:

.. literalinclude:: ../../../../examples/bgl-book/ch6_kruskal.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

The graph is constructed as an undirected weighted edge list:

.. literalinclude:: ../../../../examples/bgl-book/ch6_kruskal.cpp
   :language: cpp
   :linenos:
   :lines: 32-70
   :caption: Building the weighted graph

Kruskal's algorithm is called with a single function:

.. literalinclude:: ../../../../examples/bgl-book/ch6_kruskal.cpp
   :language: cpp
   :linenos:
   :lines: 85-99
   :caption: Running Kruskal and displaying MST

The example also shows maximum spanning tree with a custom comparator:

.. literalinclude:: ../../../../examples/bgl-book/ch6_kruskal.cpp
   :language: cpp
   :linenos:
   :lines: 101-132
   :caption: Maximum spanning tree variant

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch6_kruskal

Sample Output
------------

.. code-block:: text

   === Kruskal's Minimum Spanning Tree ===
   Based on BGL Book Chapter 6

   Graph: 7 vertices representing cities
   Edges represent possible cable routes with costs

   Input edges (sorted by weight for Kruskal):
     0 -- 3 : 5
     2 -- 4 : 5
     3 -- 5 : 6
     0 -- 1 : 7
     1 -- 4 : 7
     1 -- 2 : 8
     4 -- 5 : 8
     1 -- 3 : 9
     4 -- 6 : 9
     5 -- 6 : 11
     3 -- 4 : 15

   Minimum Spanning Tree edges:
     0 -- 3 : 5
     2 -- 4 : 5
     3 -- 5 : 6
     0 -- 1 : 7
     1 -- 4 : 7
     4 -- 6 : 9

   Total MST weight: 39
   Number of MST edges: 6
   (A tree with N vertices has N-1 edges)

   === Maximum Spanning Tree ===
   Maximum Spanning Tree edges:
     3 -- 4 : 15
     5 -- 6 : 11
     1 -- 3 : 9
     1 -- 2 : 8
     4 -- 6 : 9
     2 -- 4 : 5

   Total MaxST weight: 57

Key NWGraph Features Demonstrated
--------------------------------

- **kruskal()**: Built-in Kruskal MST implementation
- **edge_list<directedness::undirected, double>**: Undirected weighted edges
- **Custom comparators**: Changing algorithm behavior (max vs min)
- **Union-find**: Efficient component tracking (internal to algorithm)

Kruskal vs Prim
---------------

===================  ==============  ================
Property             Kruskal         Prim
===================  ==============  ================
Strategy             Edge-centric    Vertex-centric
Data structure       Union-find      Priority queue
Best for             Sparse graphs   Dense graphs
Complexity           O(E log E)      O(E log V)
Implementation       Sort edges      Grow tree
===================  ==============  ================

Both algorithms produce the same MST (or one of equal weight if ties exist).

MST Properties
--------------

A minimum spanning tree has these properties:

- **Tree**: Connected with no cycles
- **Spanning**: Contains all vertices
- **Minimum**: Sum of edge weights is minimized
- **Uniqueness**: Unique if all edge weights are distinct

For a graph with V vertices:

- MST has exactly V-1 edges
- MST is unique if all edge weights are distinct
- Multiple MSTs possible if edges have equal weights

See Also
--------

- :doc:`ch6_prim` - Alternative MST algorithm
- ``nwgraph/algorithms/kruskal.hpp`` - Kruskal implementation
- ``nwgraph/util/disjoint_set.hpp`` - Union-find data structure
