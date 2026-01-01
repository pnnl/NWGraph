.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 8: Maximum Flow
============================================

Overview
--------

This example demonstrates the Edmonds-Karp maximum flow algorithm, which is
a BFS-based implementation of the Ford-Fulkerson method. Maximum flow finds
the maximum amount of "flow" that can be pushed through a network from a
source vertex to a sink vertex while respecting edge capacities.

**Applications:**

- Network routing (maximizing data throughput)
- Bipartite matching (finding maximum matchings)
- Image segmentation (graph cuts)
- Scheduling problems
- Transportation and logistics

Algorithm Description
--------------------

The Ford-Fulkerson method repeatedly finds augmenting paths from source to
sink and pushes flow along them. The Edmonds-Karp variant uses BFS to find
the shortest augmenting path, guaranteeing polynomial time complexity.

**Key concepts:**

- **Capacity**: Maximum flow an edge can carry
- **Flow**: Current amount being pushed through an edge
- **Residual capacity**: capacity - flow (room for more flow)
- **Residual graph**: Graph with residual capacities
- **Augmenting path**: Path from source to sink in residual graph

**Algorithm steps:**

1. Initialize flow on all edges to 0
2. While augmenting path exists (BFS in residual graph):
   a. Find path capacity (minimum residual on path)
   b. Push flow along path (update flow values)
   c. Update residual graph
3. Return total flow

**Time complexity**: O(V·E²) - at most O(V·E) BFS iterations, each O(E)

NWGraph Implementation
---------------------

The example implements Edmonds-Karp with explicit residual graph management:

.. literalinclude:: ../../../../examples/bgl-book/ch8_maxflow.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

BFS to find augmenting path:

.. literalinclude:: ../../../../examples/bgl-book/ch8_maxflow.cpp
   :language: cpp
   :linenos:
   :lines: 40-80
   :caption: BFS for augmenting paths

The main Edmonds-Karp loop:

.. literalinclude:: ../../../../examples/bgl-book/ch8_maxflow.cpp
   :language: cpp
   :linenos:
   :lines: 82-130
   :caption: Edmonds-Karp algorithm

Example network setup:

.. literalinclude:: ../../../../examples/bgl-book/ch8_maxflow.cpp
   :language: cpp
   :linenos:
   :lines: 132-198
   :caption: Flow network example

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch8_maxflow

Sample Output
------------

.. code-block:: text

   === Maximum Flow (Edmonds-Karp) ===
   Based on BGL Book Chapter 8

   Flow Network:
     s (source) -> a (capacity 10)
     s -> c (capacity 10)
     a -> b (capacity 4)
     a -> c (capacity 2)
     a -> d (capacity 8)
     b -> t (capacity 10)
     c -> d (capacity 9)
     d -> b (capacity 6)
     d -> t (capacity 10)
     t (sink)

   Computing maximum flow from s to t...

   Augmenting path 1: s -> a -> b -> t (flow: 4)
   Augmenting path 2: s -> a -> d -> t (flow: 6)
   Augmenting path 3: s -> c -> d -> t (flow: 4)
   Augmenting path 4: s -> a -> c -> d -> b -> t (flow: 2)
   ...

   Maximum flow: 19

   Final flow on each edge:
     s -> a: 10/10
     s -> c: 9/10
     a -> b: 4/4
     a -> c: 0/2
     a -> d: 6/8
     ...

Key NWGraph Features Demonstrated
--------------------------------

- **BFS for path finding**: Using bfs_edge_range or manual BFS
- **Residual graph representation**: Adjacency matrix or augmented edges
- **Path reconstruction**: Using predecessor array from BFS
- **Flow tracking**: Parallel arrays for capacity and flow

Residual Graph
--------------

The residual graph has:

- Forward edges: residual = capacity - flow
- Backward edges: residual = flow (to allow "undoing" flow)

.. code-block:: cpp

   // Residual capacity from u to v
   auto residual(u, v) {
       return capacity[u][v] - flow[u][v] + flow[v][u];
   }

   // Push delta flow from u to v
   void push_flow(u, v, delta) {
       if (capacity[u][v] > 0) {
           flow[u][v] += delta;  // Forward edge
       } else {
           flow[v][u] -= delta;  // Backward edge (undo)
       }
   }

Min-Cut Max-Flow Theorem
------------------------

The maximum flow equals the minimum cut:

   The maximum value of flow from source s to sink t equals the minimum
   capacity of an s-t cut.

An s-t cut partitions vertices into two sets S (containing s) and T
(containing t). The cut capacity is the sum of capacities of edges from
S to T.

After running max-flow, the min-cut can be found by:
1. Find all vertices reachable from s in the residual graph
2. These form set S; the rest form set T
3. Edges from S to T in the original graph form the min-cut

Applications
------------

**Bipartite Matching**: Add source connected to one partition, sink to the
other, all edges capacity 1. Maximum flow = maximum matching.

**Network Reliability**: Maximum flow = number of edge-disjoint paths.

**Image Segmentation**: Pixels are vertices, edges connect neighbors.
Min-cut separates foreground from background.

See Also
--------

- :doc:`ch4_kevin_bacon` - BFS traversal
- ``nwgraph/adaptors/bfs_edge_range.hpp`` - BFS implementation
