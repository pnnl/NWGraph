.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

==============================================================
Kruskal's Minimum Spanning Tree Algorithm (BGL Book Chapter 6)
==============================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

Given an undirected, weighted graph, a **minimum spanning tree** (MST) is a subset of
edges that connects all the vertices together, without any cycles, with the minimum
possible total edge weight.

More precisely, for a graph G = (V, E), the minimum spanning tree problem is to find
an acyclic subset T of E that connects all the vertices in V and whose total weight is
minimized.

Telephone Network Planning
--------------------------

Suppose that you are responsible for setting up the telephone lines for a remote region.
The region consists of several towns and a network of roads. Setting up a telephone line
requires access for the trucks, and hence, a road alongside the route for the line.

Your budget is quite small, so building new roads is out of the question and the telephone
lines will have to go in along existing roads. Also, you would like to minimize the total
length of wire required to connect all the towns in the region.

The telephone network planning problem is a classic application of minimum spanning trees.

Kruskal's Algorithm
-------------------

Kruskal's algorithm starts with each vertex in a tree by itself and with no edges in the
set T (which will become the minimum spanning tree). The algorithm then:

1. **Sort edges** by weight in non-decreasing order
2. **Examine each edge** in sorted order
3. If the edge connects two vertices in **different trees**, merge the trees and add the
   edge to T
4. Continue until all edges have been examined

When finished, T will span the graph (assuming it's connected) and be a minimum spanning
tree.

The key data structure used by Kruskal's algorithm is the **disjoint-set** (also called
union-find), which efficiently tracks which vertices belong to the same tree and supports
merging trees.

Comparison with Prim's Algorithm
--------------------------------

+--------------------------+---------------------------+---------------------------+
| Feature                  | Kruskal's                 | Prim's                    |
+==========================+===========================+===========================+
| Approach                 | Edge-centric              | Vertex-centric            |
+--------------------------+---------------------------+---------------------------+
| Data structure           | Disjoint-set (union-find) | Priority queue            |
+--------------------------+---------------------------+---------------------------+
| Better for               | Sparse graphs             | Dense graphs              |
+--------------------------+---------------------------+---------------------------+
| Time complexity          | O(E log E)                | O(E log V) with heap      |
+--------------------------+---------------------------+---------------------------+

Note that minimum spanning trees are not unique; there can be more than one MST for a
particular graph if multiple edges have the same weight.

NWGraph Implementation
---------------------

NWGraph provides a ``kruskal`` function that implements Kruskal's algorithm using the
disjoint-set data structure.

.. literalinclude:: ../../../../examples/bgl-book/ch6_kruskal.cpp
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
   make ch6_kruskal

Then run the example:

.. code-block:: bash

   ./examples/bgl-book/ch6_kruskal

The program uses a sample road network and computes the minimum spanning tree, which represents
the optimal layout for telephone lines.

Sample Output
------------

.. code-block:: text

   Telephone Network MST (Kruskal's Algorithm)

   MST edges:
   Nobel -- Parry Sound (3 miles)
   Novar -- Huntsville (5 miles)
   Rosseau -- Sprucedale (8 miles)
   ...

   Total wire required: 145 miles

Key NWGraph Features Demonstrated
--------------------------------

- **Undirected weighted graphs**: Road networks with distances
- **Edge sorting**: Processing edges by weight
- **Disjoint-set data structure**: Tracking connected components
- **Edge list output**: Collecting MST edges

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 21.1: Growing a Minimum Spanning Tree** and **Chapter 21.2: Kruskal's and Prim's Algorithms**
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 6.3

See Also
--------

- :doc:`ch6_prim` - Prim's MST algorithm (vertex-centric approach)
- :doc:`ch5_dijkstra` - Dijkstra's shortest paths (related algorithm)
- :doc:`index` - BGL Book examples overview
