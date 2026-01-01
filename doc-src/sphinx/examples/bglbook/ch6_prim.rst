.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 6.2: Prim's Minimum Spanning Tree Algorithm
============================================

Overview
--------

This example demonstrates Prim's algorithm for finding a minimum spanning
tree (MST). Unlike Kruskal's edge-centric approach, Prim's algorithm grows
the MST from a starting vertex, always adding the minimum-weight edge that
connects the tree to a new vertex.

Prim's algorithm is particularly efficient for dense graphs when implemented
with a Fibonacci heap.

Algorithm Description
--------------------

Prim's algorithm maintains a growing tree and repeatedly adds the
minimum-weight edge that crosses the "cut" between tree vertices and
non-tree vertices.

**Algorithm steps:**

1. Start with an arbitrary vertex in the tree
2. Initialize a priority queue with edges from the starting vertex
3. While tree doesn't span all vertices:
   a. Extract minimum-weight edge (u, v) from queue where v not in tree
   b. Add v to tree, add edge (u, v) to MST
   c. Add all edges from v to non-tree vertices to queue

**Time complexity**:

- With binary heap: O((V + E) log V)
- With Fibonacci heap: O(E + V log V)

NWGraph Implementation
---------------------

The example implements Prim's algorithm using NWGraph's adjacency structure
and a priority queue:

.. literalinclude:: ../../../../examples/bgl-book/ch6_prim.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

The algorithm maintains a priority queue of candidate edges:

.. literalinclude:: ../../../../examples/bgl-book/ch6_prim.cpp
   :language: cpp
   :linenos:
   :lines: 40-90
   :caption: Prim's algorithm implementation

The main function demonstrates the algorithm on a sample graph:

.. literalinclude:: ../../../../examples/bgl-book/ch6_prim.cpp
   :language: cpp
   :linenos:
   :lines: 92-147
   :caption: Running Prim's algorithm

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch6_prim

Sample Output
------------

.. code-block:: text

   === Prim's Minimum Spanning Tree ===
   Based on BGL Book Chapter 6

   Graph: 7 vertices representing locations
   Edges represent connections with costs

   Starting Prim's algorithm from vertex 0...

   MST edges in order added:
     0 -- 3 (weight: 5)
     3 -- 5 (weight: 6)
     0 -- 1 (weight: 7)
     1 -- 4 (weight: 7)
     4 -- 2 (weight: 5)
     4 -- 6 (weight: 9)

   Total MST weight: 39
   Number of edges: 6

Key NWGraph Features Demonstrated
--------------------------------

- **adjacency<0, double>**: Weighted adjacency for efficient neighbor access
- **Priority queue**: Standard C++ heap for edge selection
- **In-tree tracking**: Boolean vector for O(1) membership check
- **Edge iteration**: ``for (auto&& [v, w] : G[u])`` pattern

Cut Property
------------

Prim's algorithm relies on the **cut property** of MSTs:

   For any cut of the graph (partition into two sets), the minimum-weight
   edge crossing the cut is in some MST.

At each step, Prim's considers the cut between tree vertices and non-tree
vertices, selecting the minimum crossing edge.

Implementation Details
----------------------

The priority queue stores edges as (weight, source, target) tuples,
with the minimum weight at the top:

.. code-block:: cpp

   // Min-heap of (weight, from_vertex, to_vertex)
   using Edge = std::tuple<double, size_t, size_t>;
   std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;

   // Add edges from starting vertex
   for (auto&& [v, w] : G[start]) {
       pq.push({w, start, v});
   }

   while (!pq.empty()) {
       auto [weight, u, v] = pq.top();
       pq.pop();

       if (in_tree[v]) continue;  // Skip if already in tree

       in_tree[v] = true;
       mst_edges.push_back({u, v, weight});

       // Add edges from newly added vertex
       for (auto&& [next, w] : G[v]) {
           if (!in_tree[next]) {
               pq.push({w, v, next});
           }
       }
   }

Comparison with Dijkstra
------------------------

Prim's algorithm looks similar to Dijkstra's, but they solve different
problems:

===================  ==============  ================
Aspect               Prim            Dijkstra
===================  ==============  ================
Goal                 Min spanning    Shortest paths
Key value            Edge weight     Path distance
Update rule          min(weight)     min(dist + weight)
Result               Tree            Shortest path tree
===================  ==============  ================

See Also
--------

- :doc:`ch6_kruskal` - Alternative MST algorithm
- :doc:`ch5_dijkstra` - Similar algorithm for shortest paths
- ``nwgraph/adjacency.hpp`` - Adjacency container
