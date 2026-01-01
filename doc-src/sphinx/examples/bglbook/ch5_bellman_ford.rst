.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 5.3: Bellman-Ford Algorithm and Distance Vector Routing
============================================

Overview
--------

This example demonstrates the Bellman-Ford single-source shortest-paths
algorithm applied to a router network. The Bellman-Ford algorithm can
handle graphs with negative edge weights (unlike Dijkstra) and can detect
negative cycles.

The algorithm is the basis for distance-vector routing protocols like RIP
(Routing Information Protocol), where routers periodically exchange distance
estimates with neighbors rather than maintaining a complete topology.

Algorithm Description
--------------------

The Bellman-Ford algorithm works by repeatedly relaxing all edges. After
|V|-1 iterations, all shortest paths are found (if no negative cycles exist).
A final iteration checks for negative cycles.

**Algorithm steps:**

1. Initialize distance[source] = 0, distance[others] = ∞
2. Repeat |V|-1 times:
   - For each edge (u, v, weight):
     - If distance[u] + weight < distance[v]:
       - distance[v] = distance[u] + weight
       - predecessor[v] = u
3. Check for negative cycles:
   - For each edge (u, v, weight):
     - If distance[u] + weight < distance[v]:
       - Negative cycle exists!

**Time complexity**: O(V·E)

**Advantage over Dijkstra**: Handles negative edge weights

NWGraph Implementation
---------------------

The example implements Bellman-Ford directly on an edge list, which is
natural for this algorithm since it iterates over all edges:

.. literalinclude:: ../../../../examples/bgl-book/ch5_bellman_ford.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

The algorithm implementation shows edge relaxation:

.. literalinclude:: ../../../../examples/bgl-book/ch5_bellman_ford.cpp
   :language: cpp
   :linenos:
   :lines: 48-94
   :caption: Bellman-Ford implementation

The router network is constructed with transmission delays as weights:

.. literalinclude:: ../../../../examples/bgl-book/ch5_bellman_ford.cpp
   :language: cpp
   :linenos:
   :lines: 96-134
   :caption: Building the router network

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch5_bellman_ford

Sample Output
------------

.. code-block:: text

   === Bellman-Ford Algorithm and Distance Vector Routing ===
   Based on BGL Book Chapter 5.3

   Router network with transmission delays:
     A connects to B (5.0) and C (1.0)
     B connects to D (1.3) and E (3.0)
     C connects to E (10.0) and F (2.0)
     D connects to E (0.4) and H (6.3)
     E connects to H (1.3)
     F connects to G (1.2)
     G connects to H (0.5)

   Computing shortest paths from router A...

   Shortest path distances and predecessors:
   ----------------------------------------
     A: 0  (via A)
     B: 5  (via A)
     C: 1  (via A)
     D: 6.3  (via B)
     E: 6.7  (via D)
     F: 3  (via C)
     G: 4.2  (via F)
     H: 4.7  (via G)

   Shortest path from A to H:
     A -> C -> F -> G -> H
     Total delay: 4.7

   === Example with Negative Cycle ===
   Negative cycle detected! (as expected)
   The cycle 1 -> 2 -> 3 -> 1 has total weight -3

   Note: Bellman-Ford is O(V*E), slower than Dijkstra's O((V+E)log V),
   but can handle negative weights and detect negative cycles.

Key NWGraph Features Demonstrated
--------------------------------

- **edge_list<directedness::directed, double>**: Weighted directed edge list
- **Iterating over edges**: ``for (auto&& edge : edges)``
- **Structured bindings**: ``auto&& [u, v, w] = edge``
- **Lambda functions**: Custom weight extraction

Negative Cycle Detection
------------------------

A negative cycle is a cycle where the sum of edge weights is negative.
If reachable from the source, it makes shortest paths undefined (we can
always go around the cycle again to get a shorter path).

.. code-block:: cpp

   // After |V|-1 iterations, check for negative cycles
   for (auto&& edge : edges) {
       auto u = std::get<0>(edge);
       auto v = std::get<1>(edge);
       auto w = weight_fn(edge);

       if (distance[u] + w < distance[v]) {
           // Negative cycle detected!
           return false;
       }
   }

Distance Vector Routing
-----------------------

In distance-vector routing (e.g., RIP):

1. Each router maintains a distance vector to all destinations
2. Routers periodically share their vectors with neighbors
3. Receiving routers update distances using Bellman-Ford relaxation
4. Eventually, all routers converge to correct shortest paths

The Bellman-Ford algorithm models this distributed computation in a
centralized way.

See Also
--------

- :doc:`ch5_dijkstra` - Faster algorithm for non-negative weights
- ``nwgraph/edge_list.hpp`` - Edge list container
