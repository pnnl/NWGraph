.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=======================================================================
Strongly Connected Components and Web Page Links (BGL Book Chapter 7.3)
=======================================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

In a directed graph, groups of vertices that are mutually reachable are called
**strongly connected components** (SCCs). A strongly connected component is a maximal
set of vertices where every vertex is reachable from every other vertex *following the
direction of edges*.

This is different from connected components in undirected graphs, where edge direction
is ignored.

Web Page Link Analysis
----------------------

The World Wide Web can be naturally represented as a directed graph:

* Each **web page** is a vertex
* Each **hyperlink** from page A to page B is a directed edge

Our goal is to compute the strongly connected components of this web graph. Pages that
link to each other (directly or indirectly) form a strongly connected component.

A study of 200 million Web pages found that **56 million** pages form one large strongly
connected component, demonstrating that a significant portion of the web is highly
interconnected.

Tarjan's Algorithm
------------------

The BGL implements **Tarjan's algorithm** for computing strongly connected components
in linear time O(|V| + |E|). The algorithm uses a single depth-first search traversal
and maintains two key pieces of information for each vertex:

1. **Discovery time** (d[v]): When the vertex was first discovered
2. **Low-link value**: The smallest discovery time reachable from the subtree rooted at v

The algorithm identifies SCC boundaries by detecting when a vertex's low-link value
equals its discovery time, indicating it's the "root" of an SCC.

Mutually Reachable Relation
---------------------------

The mutually reachable relation for directed graphs is an equivalence relation:

* **Reflexive**: Every vertex is reachable from itself
* **Symmetric**: If u can reach v and v can reach u, they are mutually reachable
* **Transitive**: If u↔v and v↔w, then u↔w

Strongly connected components are therefore equivalence classes under the mutually
reachable relation, and they partition the vertices of a directed graph into disjoint
subsets.

NWGraph Implementation
---------------------

NWGraph provides a ``strong_components`` function that implements Tarjan's algorithm.

.. literalinclude:: ../../../../examples/bgl-book/ch7_strongly_connected.cpp
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
   make ch7_strongly_connected

Then run the example:

.. code-block:: bash

   ./examples/bgl-book/ch7_strongly_connected

The program uses a sample graph of web pages connected by URL links and identifies the
strongly connected components.

Sample Output
------------

.. code-block:: text

   Web Page Links - Strongly Connected Components

   Found 6 strongly connected components

   SCC 0: www.boost.org, sourceforge.net (mutually linked open source sites)
   SCC 1: www.lsc.nd.edu, www.lam-mpi.org (HPC-related sites)
   SCC 2: anubis.dkuug.dk
   SCC 3: www.hp.com
   SCC 4: www.yahoogroups.com, weather.yahoo.com (Yahoo properties)
   SCC 5: nytimes.com, www.boston.com (news sites)

Key NWGraph Features Demonstrated
--------------------------------

- **Directed graphs**: Web links are naturally directional
- **Tarjan's algorithm**: Linear-time SCC computation
- **DFS with low-link values**: Tracking reachability information
- **Component labeling**: Mapping vertices to their SCC

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 20.5: Strongly Connected Components**
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 7.3

See Also
--------

- :doc:`ch7_connected` - Connected components for undirected graphs
- :doc:`ch4_loop_detection` - DFS for cycle detection
- :doc:`index` - BGL Book examples overview
