.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

===========================================================
Prim's Minimum Spanning Tree Algorithm (BGL Book Chapter 6)
===========================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

Prim's algorithm is another classic algorithm for finding a minimum spanning tree (MST)
of an undirected weighted graph. Unlike Kruskal's algorithm which works edge by edge,
Prim's algorithm grows the MST one **vertex** at a time.

Telephone Network Planning
--------------------------

This example applies Prim's algorithm to the same telephone network planning problem
as in the Kruskal example. The goal is to find the minimum cost way to connect all
towns in a region using existing roads.

Prim's Algorithm
----------------

Prim's algorithm grows the minimum spanning tree one vertex at a time. The basic idea
is to add vertices to the MST based on which of the remaining vertices shares an edge
having minimum weight with any of the vertices already in the tree.

The algorithm proceeds as follows:

1. Start with an arbitrary vertex as the initial tree
2. Find the minimum weight edge connecting a vertex in the tree to a vertex outside
3. Add that edge and vertex to the tree
4. Repeat until all vertices are in the tree

Relationship to Dijkstra's Algorithm
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prim's algorithm is remarkably similar to Dijkstra's shortest-paths algorithm. In fact,
the BGL implementation of Prim's algorithm is simply a call to Dijkstra's algorithm,
with a special choice for the distance comparison and combine functions.

The key difference:

- **Dijkstra**: ``d[v] = min(d[u] + w(u,v), d[v])`` (accumulate path weight)
- **Prim**: ``d[v] = min(w(u,v), d[v])`` (only consider edge weight)

Output Format
-------------

Prim's algorithm outputs the MST as a **predecessor map**. For each vertex v in the
graph, ``parent[v]`` is the parent of v in the minimum spanning tree. This representation
is more compact than storing edges explicitly.

If ``parent[v] == v``, then either v is the root of the tree or it was not in the same
connected component as the rest of the vertices.

Non-Uniqueness of MSTs
----------------------

Note that the MST produced by Prim's algorithm may be slightly different from the one
produced by Kruskal's algorithm when multiple edges have the same weight. This
highlights the fact that minimum spanning trees are not unique; there can be more than
one MST for a particular graph (though they all have the same total weight).

NWGraph Implementation
---------------------

NWGraph provides a ``prim`` function that implements Prim's algorithm using a priority
queue to efficiently select the minimum weight edge.

.. literalinclude:: ../../../../examples/bgl-book/ch6_prim.cpp
   :language: cpp
   :linenos:
   :caption: Complete source code

Running the Example
------------------

Build and run the example:

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch6_prim

The program computes the MST starting from an arbitrary vertex and outputs the
predecessor map representing the tree.

Sample Output
------------

.. code-block:: text

   Telephone Network MST (Prim's Algorithm)

   Starting vertex: Nobel

   MST edges (via predecessor map):
   Parry Sound's parent: Nobel (3 miles)
   McKellar's parent: Parry Sound (9 miles)
   ...

   Total wire required: 145 miles

Key NWGraph Features Demonstrated
--------------------------------

- **Priority queue traversal**: Similar to Dijkstra's algorithm
- **Predecessor maps**: Recording the MST structure
- **Weighted undirected graphs**: Road networks with distances
- **Greedy algorithm**: Always selecting the minimum weight edge

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 21.2: Kruskal's and Prim's Algorithms**
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 6.4

See Also
--------

- :doc:`ch6_kruskal` - Kruskal's MST algorithm (edge-centric approach)
- :doc:`ch5_dijkstra` - Dijkstra's shortest paths (similar algorithm structure)
- :doc:`index` - BGL Book examples overview
