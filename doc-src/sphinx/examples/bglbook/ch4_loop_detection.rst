.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

===================================================================
Finding Loops in Program Control-Flow Graphs (BGL Book Chapter 4.2)
===================================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

One important assumption when using topological sorting is that the dependency graph does not
have any cycles. A graph with cycles does not have a topological ordering. A well-formed
makefile will have no cycles, but errors do occur, and our build system should be able to
catch and report such errors.

This example applies the same concept to program analysis: finding loops in control-flow
graphs. Each box in a control-flow graph represents a basic block of instructions, and
edges represent possible control flow between blocks.

Detecting Cycles with DFS
-------------------------

Depth-first search can be used for detecting cycles. If DFS is applied to a graph that has
a cycle, then one of the branches of a DFS tree will loop back on itself. That is, there
will be an edge from a vertex to one of its ancestors in the tree. This kind of edge is
called a **back edge**.

This occurrence can be detected using a three-way coloring scheme instead of simple
visited/not-visited marking:

* **White** means undiscovered
* **Gray** means discovered but still searching descendants
* **Black** means the vertex and all of its descendants have been discovered

A cycle in the graph is then identified by an adjacent vertex that is gray, meaning that
an edge is looping back to an ancestor.

Finding Loops in Control-Flow Graphs
------------------------------------

Finding the loops in a control-flow graph consists of two steps:

1. **Find all back edges**: Each back edge (u, v) identifies a loop, since v is the
   ancestor of u in the DFS tree and adding (u, v) completes the loop. The vertex v
   is called the *loop head*. DFS is used to identify the back edges.

2. **Determine which vertices belong to each loop**: For a vertex v to belong to a loop
   indicated by a back edge (t, h), v must be reachable from h (the loop head) and t
   (the loop tail) must be reachable from v.

The Algorithm
-------------

The algorithm uses DFS to find back edges by recording edges that connect a vertex to
one of its ancestors (a gray vertex). For each back edge found, we then compute the
*loop extent*—the set of all vertices that belong to that loop—by:

1. Computing all vertices reachable from the loop head
2. Computing all vertices from which you can reach the loop tail
3. Intersecting these two sets

NWGraph Implementation
---------------------

NWGraph provides the ``back_edge_range`` adaptor that yields back edges during a DFS
traversal, making it easy to detect cycles and find loops.

.. literalinclude:: ../../../../examples/bgl-book/ch4_loop_detection.cpp
   :language: cpp
   :linenos:
   :caption: Complete source code

Running the Example
------------------

Build and run the example:

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch4_loop_detection

The program constructs a control-flow graph and identifies all back edges (loops) in
the graph.

Sample Output
------------

.. code-block:: text

   Control-flow graph loop detection
   Found back edge: B7 -> B1 (indicates a loop)
   Loop head: B1
   Loop vertices: B1, B2, B4, B5, B7

Key NWGraph Features Demonstrated
--------------------------------

- **Directed graphs**: Control-flow graphs are naturally directed
- **DFS traversal**: Using depth-first search to classify edges
- **Back edge detection**: Using ``back_edge_range`` to find cycles
- **Edge classification**: Distinguishing tree edges from back edges

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 20.3: Depth-First Search** (edge classification and back edges)
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 4.2

See Also
--------

- :doc:`ch3_toposort` - Topological sorting (requires acyclic graphs)
- :doc:`ch4_kevin_bacon` - BFS for shortest paths
- :doc:`index` - BGL Book examples overview
