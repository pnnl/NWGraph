.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=====================================================
Boost Graph Library Examples (Rewritten for NW Graph)
=====================================================

These examples are adapted from "The Boost Graph Library: User Guide and
Reference Manual" by Jeremy G. Siek, Lie-Quan Lee, and Andrew Lumsdaine
(Addison-Wesley, 2002). They demonstrate how NWGraph implements classic
graph algorithms using modern C++20 idioms.

Each example includes the complete source code with detailed explanations
matching the pedagogical approach of the original book.

.. toctree::
   :maxdepth: 1
   :caption: Chapter Examples

   ch3_toposort
   ch4_kevin_bacon
   ch4_loop_detection
   ch5_dijkstra
   ch5_bellman_ford
   ch6_kruskal
   ch6_prim
   ch7_connected
   ch7_strongly_connected
   ch8_maxflow
   ch9_knights_tour

Overview
--------

Chapter 3: File Dependencies (Topological Sort)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Demonstrates topological sorting using DFS on a makefile dependency graph.
Topological sort orders vertices so that for every edge (u,v), vertex u
comes before v in the ordering.

**Key concepts**: DFS, DAG, topological ordering

Chapter 4: Graph Search
^^^^^^^^^^^^^^^^^^^^^^^

**4.1 Six Degrees of Kevin Bacon**

Uses BFS to compute "Bacon numbers" - the number of connections from any
actor to Kevin Bacon through co-starring in movies.

**Key concepts**: BFS, shortest paths in unweighted graphs, bipartite graphs

**4.2 Loop Detection in Control-Flow Graphs**

Uses DFS with three-coloring to detect back edges (cycles) in directed graphs.
Back edges in a control-flow graph indicate loops.

**Key concepts**: DFS, back edges, cycle detection, control-flow analysis

Chapter 5: Shortest Paths
^^^^^^^^^^^^^^^^^^^^^^^^^

**5.3 Bellman-Ford Algorithm**

Demonstrates the Bellman-Ford single-source shortest-paths algorithm applied
to router networks (distance-vector routing). Can handle negative edge weights
and detect negative cycles.

**Key concepts**: Edge relaxation, negative weights, negative cycle detection

**5.4 Dijkstra's Algorithm (OSPF Routing)**

Uses Dijkstra's algorithm for shortest paths in an OSPF network topology.
OSPF is a link-state routing protocol where routers compute shortest paths
to build routing tables.

**Key concepts**: Priority queue, greedy algorithm, link-state routing

Chapter 6: Minimum Spanning Trees
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**6.1 Kruskal's Algorithm**

Demonstrates Kruskal's MST algorithm by sorting edges by weight and using
a union-find data structure to avoid cycles.

**Key concepts**: Union-find, edge sorting, greedy algorithm

**6.2 Prim's Algorithm**

Demonstrates Prim's MST algorithm by growing the tree from a starting vertex,
always adding the minimum-weight edge to the tree.

**Key concepts**: Priority queue, cut property, growing a tree

Chapter 7: Connected Components
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**7.1 Connected Components**

Uses BFS to find connected components in an undirected graph. Each component
is a maximal subgraph where every vertex is reachable from every other.

**Key concepts**: BFS, component labeling, graph connectivity

**7.2 Strongly Connected Components**

Demonstrates Tarjan's algorithm for finding strongly connected components
in a directed graph, with application to web page analysis.

**Key concepts**: Tarjan's algorithm, lowlink values, web graph analysis

Chapter 8: Maximum Flow
^^^^^^^^^^^^^^^^^^^^^^^

Implements the Edmonds-Karp maximum flow algorithm (BFS-based Ford-Fulkerson).
Maximum flow finds the maximum amount that can be pushed through a network
from source to sink.

**Key concepts**: Augmenting paths, residual graph, flow networks

Chapter 9: Implicit Graphs
^^^^^^^^^^^^^^^^^^^^^^^^^^

Solves the Knight's Tour problem using an implicit graph representation.
The graph structure is computed on-the-fly from the knight's movement rules
rather than stored explicitly.

**Key concepts**: Implicit graphs, backtracking, Warnsdorff's heuristic,
Hamiltonian path

Building the Examples
---------------------

The BGL book examples are built as part of the NWGraph examples:

.. code-block:: bash

   # From the NWGraph root directory
   mkdir -p build && cd build
   cmake .. -DNWGRAPH_BUILD_EXAMPLES=ON
   make -j8

   # Or build individual examples
   make ch3_toposort ch4_kevin_bacon ch5_dijkstra -j8

Executables are placed in ``build/examples/bgl-book/``.

Running the Examples
--------------------

Each example can be run directly from the build directory:

.. code-block:: bash

   # Run topological sort example
   ./examples/bgl-book/ch3_toposort

   # Run Kevin Bacon example
   ./examples/bgl-book/ch4_kevin_bacon

   # Run Dijkstra shortest paths example
   ./examples/bgl-book/ch5_dijkstra

   # Run Kruskal MST example
   ./examples/bgl-book/ch6_kruskal

Most examples use built-in test data and do not require external input files.
See individual example pages for specific usage details.

Reference
---------

Siek, J. G., Lee, L.-Q., & Lumsdaine, A. (2002). *The Boost Graph Library:
User Guide and Reference Manual*. Addison-Wesley Professional.
