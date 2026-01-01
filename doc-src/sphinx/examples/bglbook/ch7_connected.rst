.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 7.1: Connected Components
============================================

Overview
--------

This example demonstrates finding connected components in an undirected
graph using BFS. A connected component is a maximal subgraph where every
vertex is reachable from every other vertex.

**Applications:**

- Network analysis (finding isolated clusters)
- Image segmentation (labeling connected regions)
- Social network analysis (finding communities)
- Detecting disconnected systems

Algorithm Description
--------------------

The algorithm uses BFS to explore each component:

**Algorithm steps:**

1. Initialize all vertices as unassigned
2. For each unassigned vertex v:
   a. Start a new component
   b. Perform BFS from v, assigning all reached vertices to this component
   c. Increment component counter

**Time complexity**: O(V + E) - each vertex and edge is visited once

**Space complexity**: O(V) for component labels and BFS queue

NWGraph Implementation
---------------------

The example implements connected components using NWGraph's adjacency
structure and BFS:

.. literalinclude:: ../../../../examples/bgl-book/ch7_connected.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

The BFS-based component finder:

.. literalinclude:: ../../../../examples/bgl-book/ch7_connected.cpp
   :language: cpp
   :linenos:
   :lines: 39-72
   :caption: BFS connected components implementation

The main function demonstrates the algorithm:

.. literalinclude:: ../../../../examples/bgl-book/ch7_connected.cpp
   :language: cpp
   :linenos:
   :lines: 74-135
   :caption: Example graphs with multiple components

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch7_connected

Sample Output
------------

.. code-block:: text

   === Connected Components ===
   Based on BGL Book Chapter 7

   Graph structure:
     Component 0: 0 - 1 - 2 - 0 (triangle)
     Component 1: 3 - 4 (edge)
     Component 2: 5 (isolated vertex)
     Component 3: 6 - 7 - 8 (path)

   Vertex assignments:
     Vertex 0 -> Component 0
     Vertex 1 -> Component 0
     Vertex 2 -> Component 0
     Vertex 3 -> Component 1
     Vertex 4 -> Component 1
     Vertex 5 -> Component 2
     Vertex 6 -> Component 3
     Vertex 7 -> Component 3
     Vertex 8 -> Component 3

   Number of connected components: 4

   Component details:
     Component 0 (size 3): {0, 1, 2}
     Component 1 (size 2): {3, 4}
     Component 2 (size 1): {5}
     Component 3 (size 3): {6, 7, 8}

   === Fully Connected Graph ===
   Complete graph K5 has 1 component(s)
   All 5 vertices belong to component 0

Key NWGraph Features Demonstrated
--------------------------------

- **adjacency<0>**: Adjacency list for undirected graph traversal
- **BFS traversal**: Level-by-level exploration
- **edge_list<directedness::undirected>**: Undirected edge representation
- **Component labeling**: Assigning IDs to connected regions

Connected vs Strongly Connected
-------------------------------

===================  ====================  =====================
Property             Connected (undirected) Strongly Connected
===================  ====================  =====================
Graph type           Undirected            Directed
Definition           Path exists (any dir)  Path exists (both dirs)
Algorithm            BFS/DFS               Tarjan's/Kosaraju's
Complexity           O(V + E)              O(V + E)
===================  ====================  =====================

For undirected graphs, there's only one notion of connectivity. For directed
graphs, we distinguish between:

- **Weakly connected**: Connected when ignoring edge direction
- **Strongly connected**: Path exists in both directions between any two vertices

Parallel Connected Components
-----------------------------

NWGraph also supports parallel connected components algorithms using
label propagation or Shiloach-Vishkin. These are useful for large graphs:

.. code-block:: cpp

   #include "nwgraph/algorithms/connected_components.hpp"

   // Parallel connected components
   auto labels = connected_components(G);

Implementation Pattern
----------------------

The frontier-based BFS pattern is common in graph algorithms:

.. code-block:: cpp

   std::vector<size_t> frontier, next_frontier;
   frontier.push_back(start);

   while (!frontier.empty()) {
       next_frontier.clear();
       for (auto u : frontier) {
           for (auto&& [v] : G[u]) {
               if (!visited[v]) {
                   visited[v] = true;
                   next_frontier.push_back(v);
               }
           }
       }
       frontier = std::move(next_frontier);
   }

See Also
--------

- :doc:`ch7_strongly_connected` - Strongly connected components
- :doc:`ch4_kevin_bacon` - BFS for shortest paths
- ``nwgraph/algorithms/connected_components.hpp`` - Built-in implementation
