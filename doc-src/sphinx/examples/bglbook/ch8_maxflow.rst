.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=================================
Maximum Flow (BGL Book Chapter 8)
=================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

The **maximum flow problem** is the problem of determining how much of some quantity
(say, water, data, or goods) can move through a network from a source to a sink.

There is a long history of algorithms for solving the maximum flow problem, with the
first algorithm due to Ford and Fulkerson. The best general-purpose algorithms known
include the Edmonds-Karp algorithm (a refinement of Ford-Fulkerson) and the push-relabel
algorithm of Goldberg.

Definitions
-----------

A **flow network** is a directed graph G = (V, E) with:

* A **source vertex** s (where flow originates)
* A **sink vertex** t (where flow is collected)
* Each edge has a positive real-valued **capacity**

The flow function f must satisfy three constraints:

1. **Capacity constraint**: f(u, v) <= c(u, v) for all edges
2. **Skew symmetry**: f(u, v) = -f(v, u) for all vertex pairs
3. **Flow conservation**: Flow into a vertex equals flow out (except for s and t)

The **maximum flow** is the maximum possible value of net flow entering the sink vertex t.

Max-Flow Min-Cut Theorem
------------------------

An important property of a flow network is that the maximum flow is related to the
capacity of the narrowest part of the network. According to the **Max-Flow Min-Cut
Theorem**, the maximum value of the flow from source s to sink t equals the minimum
capacity among all (S, T) cuts.

An (S, T) cut is a separation of the graph's vertices into two sets S and T, where s
is in S and t is in T. The capacity of a cut is the sum of the capacities of edges
crossing from S to T.

Edge Connectivity
-----------------

Whether an engineer is designing a telephone network, a LAN for a large company, or
the router connections for the Internet backbone, an important consideration is how
resilient the network is to damage.

**Edge connectivity** is the minimum number of edges that can be cut to produce a
graph with two disconnected components. This can be computed using maximum flow:
set the capacity of every edge to one, and the maximum flow between any two vertices
gives the minimum number of edges separating them.

NWGraph Implementation
---------------------

NWGraph provides maximum flow algorithms including the Edmonds-Karp algorithm.

.. literalinclude:: ../../../../examples/bgl-book/ch8_maxflow.cpp
   :language: cpp
   :linenos:
   :caption: Complete source code

Running the Example
------------------

Build and run the example:

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch8_maxflow

The program computes the maximum flow through a network and identifies the minimum cut.

Sample Output
------------

.. code-block:: text

   Flow Network - Maximum Flow

   Source: A, Sink: H

   Maximum flow: 7 units

   Flow on each edge:
   A -> B: 3/4
   A -> C: 4/5
   B -> D: 3/3
   ...

   Minimum cut edges: {(B,D), (C,F)}

Key NWGraph Features Demonstrated
--------------------------------

- **Directed graphs with capacities**: Modeling flow networks
- **Augmenting path algorithms**: Ford-Fulkerson method
- **Residual graphs**: Tracking remaining capacity
- **Min-cut computation**: Finding network bottlenecks

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 24: Maximum Flow** (Ford-Fulkerson, Edmonds-Karp, Push-Relabel)
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 8

See Also
--------

- :doc:`ch5_dijkstra` - Shortest paths (related network algorithm)
- :doc:`ch7_connected` - Connected components
- :doc:`index` - BGL Book examples overview
