.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 4.2: Loop Detection in Control-Flow Graphs
============================================

Overview
--------

This example demonstrates using DFS to detect back edges (cycles) in a
directed graph. In program analysis, control-flow graphs represent the
flow of execution through a program, and back edges indicate loops.

Detecting loops is essential for:

- Compiler optimizations (loop unrolling, vectorization)
- Program analysis (termination checking, resource usage)
- Security analysis (infinite loop detection)

Algorithm Description
--------------------

The algorithm uses DFS with three-way vertex coloring:

- **White**: Vertex has not been visited
- **Grey**: Vertex is currently being explored (on the DFS stack)
- **Black**: Vertex and all its descendants have been fully explored

A **back edge** is an edge from a vertex to one of its ancestors in the
DFS tree - i.e., an edge to a grey vertex. Back edges indicate cycles
because they create a path from a vertex back to itself.

**Algorithm steps:**

1. Initialize all vertices as white
2. For each unvisited vertex, perform DFS
3. When entering a vertex, color it grey
4. When leaving a vertex, color it black
5. If we encounter an edge to a grey vertex, it's a back edge (cycle)

**Time complexity**: O(V + E)

NWGraph Implementation
---------------------

The example implements cycle detection manually to demonstrate the
three-coloring technique:

.. literalinclude:: ../../../../examples/bgl-book/ch4_loop_detection.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

The recursive DFS function checks for back edges:

.. literalinclude:: ../../../../examples/bgl-book/ch4_loop_detection.cpp
   :language: cpp
   :linenos:
   :lines: 40-58
   :caption: DFS cycle detection

A wrapper function handles disconnected graphs:

.. literalinclude:: ../../../../examples/bgl-book/ch4_loop_detection.cpp
   :language: cpp
   :linenos:
   :lines: 60-76
   :caption: Main cycle detection function

The main function demonstrates cycle detection on several example graphs:

.. literalinclude:: ../../../../examples/bgl-book/ch4_loop_detection.cpp
   :language: cpp
   :linenos:
   :lines: 97-145
   :caption: Example: while loop control-flow graph

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch4_loop_detection

The example includes three test graphs:

1. A simple while loop (has cycle)
2. A DAG (no cycles)
3. Nested loops (multiple cycles)

Sample Output
------------

.. code-block:: text

   === Loop Detection in Control-Flow Graphs ===
   Based on BGL Book Chapter 4.2

   Graph 1: Simple while loop
     0 -> 1 -> 2 -> 1 (loop back)
               |
               v
               3
   Has cycle: yes
   Back edges found: 1
     2 -> 1 (loop)

   Graph 2: DAG (no loops)
     0 -> 1 -> 3
     |    |
     v    v
     2 -> 3
   Has cycle: no

   Graph 3: Nested loops
     0 -> 1 -> 2 -> 3 -> 2 (inner loop)
          ^         |
          +---------+ (outer loop)
   Has cycle: yes
   Back edges found: 2
     3 -> 2 (loop)
     3 -> 1 (loop)

Key NWGraph Features Demonstrated
--------------------------------

- **edge_list<directedness::directed>**: Directed graph representation
- **adjacency<0>**: Forward adjacency for DFS traversal
- **Structured bindings**: ``for (auto&& [v] : G[u])`` pattern
- **Manual algorithm implementation**: NWGraph makes it easy to implement
  custom algorithms using standard C++ patterns

Control-Flow Graph Representation
---------------------------------

Control-flow graphs can be represented directly with NWGraph:

.. code-block:: cpp

   // Vertices: basic blocks (0=entry, 1=condition, 2=body, 3=exit)
   edge_list<directedness::directed> cfg(4);
   cfg.push_back(0, 1);  // entry -> condition
   cfg.push_back(1, 2);  // condition -> body (true branch)
   cfg.push_back(1, 3);  // condition -> exit (false branch)
   cfg.push_back(2, 1);  // body -> condition (back edge - loop!)

   adjacency<0> G(cfg);

Edge Classification
-------------------

DFS classifies edges into four types:

- **Tree edges**: Edges in the DFS spanning tree
- **Back edges**: Edges to ancestors (indicate cycles)
- **Forward edges**: Edges to descendants (not in tree)
- **Cross edges**: Edges to neither ancestors nor descendants

Only back edges indicate cycles in the graph.

See Also
--------

- :doc:`ch3_toposort` - DFS for topological sorting
- :doc:`ch7_strongly_connected` - Cycle-related SCC algorithm
- ``nwgraph/adaptors/dfs_range.hpp`` - DFS range adaptor
