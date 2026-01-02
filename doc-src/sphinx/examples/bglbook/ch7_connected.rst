.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=========================================
Connected Components (BGL Book Chapter 7)
=========================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

One basic question about a network is which vertices are reachable from one another.
For example, a well-designed Web site should have enough links between Web pages so
that all pages can be reached from the home page.

A **connected component** is a group of vertices in an undirected graph that are
reachable from one another. In a directed graph, groups of vertices that are *mutually*
reachable are called **strongly connected components**.

Internet Connectivity Study
---------------------------

A study of 200 million Web pages has shown interesting connectivity properties:

* **56 million** Web pages form one large strongly connected component
* When viewed as an undirected graph, **150 million** pages are in one large connected
  component
* About **50 million** pages are disconnected from the large component (they reside in
  much smaller connected components of their own)

This demonstrates how connected components analysis can reveal the structure of large
networks.

Definitions
-----------

A **path** is a sequence of vertices where there is an edge connecting each vertex to
the next vertex in the path. If there exists a path from vertex u to w, then we say
that vertex w is **reachable** from vertex u.

The reachable relation for undirected graphs is an **equivalence relation**: it is
reflexive, symmetric, and transitive. Connected components are therefore equivalence
classes with respect to the reachable relation, and they partition the vertices of
a graph into disjoint subsets.

Algorithm
---------

Computing the connected components of an undirected graph is a straightforward
application of depth-first search:

1. Run DFS on the graph
2. Mark all vertices in the same DFS tree as belonging to the same connected component
3. Increment the component number at each "start vertex" event point

The BGL implementation calls ``depth_first_search()`` with a special visitor object
that labels each discovered vertex with the current component number.

NWGraph Implementation
---------------------

NWGraph provides a ``connected_components`` function that computes all connected
components using DFS.

.. literalinclude:: ../../../../examples/bgl-book/ch7_connected.cpp
   :language: cpp
   :linenos:
   :caption: Complete source code

Running the Example
------------------

Build and run the example:

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch7_connected

The program reads a network of Internet routers and identifies the connected components,
showing which groups of routers can reach each other.

Sample Output
------------

.. code-block:: text

   Internet Router Network - Connected Components

   Found 4 connected components

   Component 0: boston1-br1, cambridge1-nbr2, nycmny1-cr1, chicago1-nbr1
   Component 1: engr-fe21, shub-e27
   Component 2: albnxg1, teledk, gw-dkuug
   Component 3: lilac-dmc, helios, rip, ccn-nerif35, ccngw-ner-cc

Key NWGraph Features Demonstrated
--------------------------------

- **Undirected graphs**: Modeling bidirectional network connections
- **DFS-based traversal**: Using depth-first search to find components
- **Component labeling**: Assigning each vertex to its component
- **Equivalence classes**: Partitioning vertices into disjoint subsets

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 20.3: Depth-First Search** (connected components via DFS forest)
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 7.2

See Also
--------

- :doc:`ch7_strongly_connected` - Strongly connected components for directed graphs
- :doc:`ch4_loop_detection` - DFS for cycle detection
- :doc:`index` - BGL Book examples overview
