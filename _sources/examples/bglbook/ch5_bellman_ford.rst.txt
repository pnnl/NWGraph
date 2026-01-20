.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=========================================================================
Bellman-Ford Algorithm and Distance Vector Routing (BGL Book Chapter 5.3)
=========================================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

The routing problem can be modeled as a graph by associating a vertex with each router
and an edge for each direct connection between two routers. Information such as delay
and bandwidth are attached to each edge. The routing problem is now transformed into
a shortest-paths graph problem.

The ultimate goal of a routing process is to deliver packets to their destinations as
quickly as possible. There are a number of factors that determine how long a packet
takes to arrive (e.g., the number of hops along the path, transmission delay within
a router, transmission delay between routers, network bandwidth). The routing protocol
must choose the best paths between routers; this information is stored in a routing table.

Distance Vector Routing
-----------------------

Some of the first Internet routing protocols, such as the **Routing Information Protocol
(RIP)**, used a distance-vector protocol. The basic idea behind RIP is for each router
to maintain an estimate of distance to all other routers and to periodically compare
notes with its neighbors.

If a router learns of a shorter path to some destination from one of its neighbors, it
will update its distance record to that destination and change its routing table to send
packets to that destination via that neighbor. After enough time, the estimated distances
maintained in this distributed fashion by multiple routers are guaranteed to converge to
the true distance.

The Bellman-Ford Algorithm
--------------------------

RIP is a distributed form of the **Bellman-Ford shortest paths algorithm**. The principle
step in the Bellman-Ford algorithm, called **edge relaxation**, corresponds to the notion
of "comparing notes with your neighbor."

The relaxation operation applied to edge (u, v) performs the following update:

.. code-block:: text

   d[v] = min(w(u, v) + d[u], d[v])

The Bellman-Ford algorithm loops through all of the edges in a graph, applying the
relaxation operation to each. The algorithm repeats this loop |V| times, after which
it is guaranteed that the distances to each vertex have been reduced to the minimum
possible (unless there is a negative cycle in the graph).

Negative Cycle Detection
~~~~~~~~~~~~~~~~~~~~~~~~

If there is a negative cycle, then there will be edges in the graph that were not
properly minimized. To verify that all edges are minimized, the algorithm loops over
all of the edges in the graph a final time, returning ``true`` if they are minimized,
and returning ``false`` otherwise.

Comparison with Dijkstra
~~~~~~~~~~~~~~~~~~~~~~~~

+-----------------------+------------------+-------------------+
| Feature               | Bellman-Ford     | Dijkstra          |
+=======================+==================+===================+
| Time complexity       | O(V * E)         | O((V+E) log V)    |
+-----------------------+------------------+-------------------+
| Negative weights      | Yes              | No                |
+-----------------------+------------------+-------------------+
| Negative cycle detect | Yes              | No                |
+-----------------------+------------------+-------------------+
| Distributed version   | Yes (RIP)        | No                |
+-----------------------+------------------+-------------------+

NWGraph Implementation
---------------------

NWGraph provides a ``bellman_ford`` function that implements the Bellman-Ford algorithm.

.. literalinclude:: ../../../../examples/bgl-book/ch5_bellman_ford.cpp
   :language: cpp
   :linenos:
   :caption: Complete source code

Building and Running the Example
--------------------------------

First, configure and build the example:

.. code-block:: bash

   # From the NWGraph root directory
   mkdir -p build && cd build
   cmake .. -DNWGRAPH_BUILD_EXAMPLES=ON
   make ch5_bellman_ford

Then run the example:

.. code-block:: bash

   ./examples/bgl-book/ch5_bellman_ford

The program uses a sample router network and computes shortest paths using the Bellman-Ford
algorithm, demonstrating how distance-vector routing protocols work.

Sample Output
------------

.. code-block:: text

   Router Network - Bellman-Ford Shortest Paths
   Source: Router A

   Router B: distance = 5ms, via Router A
   Router C: distance = 1ms, via Router A
   Router D: distance = 6.3ms, via Router B
   ...
   No negative cycles detected.

Key NWGraph Features Demonstrated
--------------------------------

- **Edge iteration**: Processing all edges for relaxation
- **Distance tracking**: Recording shortest path distances
- **Predecessor maps**: Building the shortest-paths tree
- **Negative cycle detection**: Verifying solution validity

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 22.1: The Bellman-Ford Algorithm**
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 5.3

See Also
--------

- :doc:`ch5_dijkstra` - Dijkstra's algorithm for link-state routing
- :doc:`ch6_kruskal` - Minimum spanning trees
- :doc:`index` - BGL Book examples overview
