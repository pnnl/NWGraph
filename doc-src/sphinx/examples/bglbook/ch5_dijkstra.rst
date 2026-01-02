.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=================================================================
Internet Routing with Dijkstra's Algorithm (BGL Book Chapter 5.4)
=================================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

An important application of shortest-path algorithms is Internet packet routing. The protocols
that control how packets of information are transmitted through the Internet use shortest-path
algorithms to reduce the amount of time it takes for a packet to reach its destination.

When a computer sends a message to another using the Internet Protocol (IP), the message
contents are put into a packet. If the destination address for the packet is outside of the
local network, the packet will be sent from the originating machine to an Internet router.
The router directs packets that it receives to other routers based on its routing table.

Link-State Routing and OSPF
---------------------------

By the early 1980's there began to be concerns about the scalability of distance-vector
routing (like RIP). Two particular aspects caused problems:

* In environments where the topology of the network changes frequently, distance-vector
  routing would converge too slowly to maintain accurate distance information.
* Update messages contain distances to all nodes, so the message size grows with the
  size of the entire network.

As a result, **link-state routing** was developed. With link-state routing, each router
stores a graph representing the topology of the entire network and computes its routing
table based on the graph using Dijkstra's single-source shortest-paths algorithm. To keep
the graph up to date, routers share information about which links are "up" and which are
"down" (the link state). When connectivity changes are detected, the information is
"flooded" throughout the network in what is called a link-state advertisement.

This approach has proved to be effective and is now formalized in the **Open Shortest Path
First (OSPF)** protocol, which is currently one of the preferred interior gateway routing
protocols.

Dijkstra's Algorithm
--------------------

Dijkstra's algorithm finds all the shortest paths from the source vertex to every other
vertex by iteratively growing the set of vertices S to which it knows the shortest path.
At each step of the algorithm:

1. The vertex in V - S with the smallest distance label is added to S
2. The out-edges of the vertex are relaxed: ``d[v] = min(w(u,v) + d[u], d[v])``
3. The algorithm loops back, processing the next vertex with the lowest distance label

The algorithm finishes when S contains all vertices reachable from the source vertex.

**Key difference from Bellman-Ford**: Dijkstra's algorithm requires non-negative edge
weights but is more efficient: O((V+E) log V) versus O(V*E) for Bellman-Ford.

NWGraph Implementation
---------------------

NWGraph provides a ``dijkstra`` function that implements Dijkstra's algorithm using
modern C++20 features.

.. literalinclude:: ../../../../examples/bgl-book/ch5_dijkstra.cpp
   :language: cpp
   :linenos:
   :caption: Complete source code

Running the Example
------------------

Build and run the example:

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch5_dijkstra

The program computes shortest paths from a source router to all other routers in an
OSPF-style network, demonstrating how link-state routing protocols work.

Sample Output
------------

.. code-block:: text

   OSPF Router Network - Dijkstra's Shortest Paths
   Source: Router 6

   Router 1: distance = 8, next hop = Router 3
   Router 2: distance = 5, next hop = Router 3
   Router 3: distance = 2, next hop = Router 3
   ...

Key NWGraph Features Demonstrated
--------------------------------

- **Weighted directed graphs**: Router networks with transmission delays
- **Priority queue-based traversal**: Efficient vertex selection by distance
- **Predecessor tracking**: Recording the shortest-paths tree
- **Property maps**: Associating distances and predecessors with vertices

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 22.3: Dijkstra's Algorithm**
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 5.4

See Also
--------

- :doc:`ch5_bellman_ford` - Bellman-Ford algorithm for distance-vector routing
- :doc:`ch4_kevin_bacon` - BFS for unweighted shortest paths
- :doc:`index` - BGL Book examples overview
