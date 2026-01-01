.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 5.4: Internet Routing with Dijkstra's Algorithm
============================================

Overview
--------

This example demonstrates Dijkstra's single-source shortest-paths algorithm
using an OSPF (Open Shortest Path First) router network graph. OSPF is a
link-state routing protocol where each router maintains a complete topology
of the network and computes shortest paths using Dijkstra's algorithm.

Dijkstra's algorithm is one of the most fundamental graph algorithms,
finding the shortest path from a source vertex to all other vertices in
a weighted graph with non-negative edge weights.

Algorithm Description
--------------------

Dijkstra's algorithm works by maintaining a set of vertices with known
shortest distances. It repeatedly selects the vertex with the minimum
distance estimate and "relaxes" all its outgoing edges.

**Algorithm steps:**

1. Initialize distance[source] = 0, distance[others] = ∞
2. Add source to priority queue
3. While queue is not empty:
   a. Extract vertex u with minimum distance
   b. For each neighbor v of u:
      - If distance[u] + weight(u,v) < distance[v]:
        - Update distance[v]
        - Update predecessor[v] = u
        - Add/update v in priority queue

**Time complexity**: O((V + E) log V) with binary heap

**Requirement**: All edge weights must be non-negative

NWGraph Implementation
---------------------

NWGraph provides a built-in Dijkstra implementation in
``nwgraph/algorithms/dijkstra.hpp``:

.. literalinclude:: ../../../../examples/bgl-book/ch5_dijkstra.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

The example uses the built-in OSPF graph data:

.. literalinclude:: ../../../../examples/bgl-book/ch5_dijkstra.cpp
   :language: cpp
   :linenos:
   :lines: 32-55
   :caption: Loading the OSPF network

Running Dijkstra is a single function call:

.. literalinclude:: ../../../../examples/bgl-book/ch5_dijkstra.cpp
   :language: cpp
   :linenos:
   :lines: 56-88
   :caption: Running Dijkstra and displaying results

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch5_dijkstra

The example uses a built-in OSPF network topology.

Sample Output
------------

.. code-block:: text

   === Internet Routing with Dijkstra's Algorithm ===
   Based on BGL Book Chapter 5.4

   Network topology has 28 nodes:
     Routers: RT1-RT12
     Networks: N1-N15
     Host: H1

   Computing shortest paths from RT6...

   Shortest path distances from RT6:
   --------------------------------------------------
     RT1: 3
     RT2: 2
     RT3: 1
     RT4: 2
     RT5: 2
     RT6: 0
     ...

   Note: In OSPF, link costs are typically based on bandwidth.
   Routers use these shortest paths to build their routing tables.

Key NWGraph Features Demonstrated
--------------------------------

- **dijkstra<Weight>()**: Built-in Dijkstra implementation
- **adjacency<0, Weight>**: Weighted adjacency list
- **Built-in graphs**: Pre-defined graph data for examples
- **Template weight type**: Support for different numeric types

OSPF Network Topology
--------------------

The OSPF network contains:

- **Routers (RT)**: Network devices that forward packets
- **Networks (N)**: Subnets connecting routers
- **Hosts (H)**: End devices

Link costs represent inverse bandwidth - lower cost means higher bandwidth
and preferred routing.

Dijkstra vs Bellman-Ford
------------------------

===================  ==============  ================
Property             Dijkstra        Bellman-Ford
===================  ==============  ================
Time complexity      O((V+E) log V)  O(V·E)
Negative weights     No              Yes
Negative cycles      N/A             Detects
Memory               O(V)            O(V)
===================  ==============  ================

Use Dijkstra when edge weights are non-negative (most routing applications).
Use Bellman-Ford when negative weights are possible or cycle detection is
needed.

See Also
--------

- :doc:`ch5_bellman_ford` - Bellman-Ford for negative weights
- :doc:`ch4_kevin_bacon` - BFS for unweighted shortest paths
- ``nwgraph/algorithms/dijkstra.hpp`` - Dijkstra implementation
