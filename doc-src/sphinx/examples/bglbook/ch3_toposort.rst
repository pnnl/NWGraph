.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 3: File Dependencies (Topological Sort)
============================================

Overview
--------

This example demonstrates topological sorting using depth-first search (DFS)
on a makefile dependency graph. The problem comes from the classic make
utility: given a set of files with dependencies, determine an order to
compile them such that each file is compiled after all its dependencies.

Topological sort produces a linear ordering of vertices such that for every
directed edge (u,v), vertex u comes before vertex v in the ordering. This
is only possible for directed acyclic graphs (DAGs).

Algorithm Description
--------------------

The topological sort algorithm is based on depth-first search. The key
insight is that in a DFS traversal, a vertex is finished (colored black)
only after all its descendants have been finished. By recording the
finish order in reverse, we get a valid topological ordering.

**Algorithm steps:**

1. Initialize all vertices as unvisited (white)
2. For each unvisited vertex, perform DFS
3. When a vertex finishes (all neighbors explored), prepend it to the result
4. The final list is a valid topological ordering

**Time complexity**: O(V + E) where V is vertices and E is edges.

NWGraph Implementation
---------------------

NWGraph provides the ``dfs_range`` adaptor that yields vertices in DFS order.
The example reads a dependency graph from Matrix Market format and uses
DFS to visit all vertices:

.. literalinclude:: ../../../../examples/bgl-book/ch3_toposort.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

The graph is loaded from Matrix Market format and converted to an adjacency
representation for efficient traversal:

.. literalinclude:: ../../../../examples/bgl-book/ch3_toposort.cpp
   :language: cpp
   :linenos:
   :lines: 31-46
   :caption: Loading the dependency graph

The DFS traversal visits vertices in depth-first order:

.. literalinclude:: ../../../../examples/bgl-book/ch3_toposort.cpp
   :language: cpp
   :linenos:
   :lines: 48-58
   :caption: DFS traversal and output

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch3_toposort

The example requires the following data files in the working directory:

- ``makefile-dependencies.mmio`` - The dependency graph in Matrix Market format
- ``makefile-target-names.dat`` - Human-readable names for each vertex

Sample Output
------------

.. code-block:: text

   === Topological Sort for Makefile Dependencies ===
   Based on BGL Book Chapter 3

   DFS traversal order:
     zow.o
     zow.cpp
     dax.o
     dax.cpp
     bar.cpp
     bar.o
     foo.cpp
     foo.o
     zow.h
     boz.h
     ...

Key NWGraph Features Demonstrated
--------------------------------

- **dfs_range**: Lazy DFS traversal adaptor that yields vertices on demand
- **read_mm**: Reading graphs from Matrix Market format
- **adjacency<0>**: Forward adjacency list representation
- **edge_list**: Intermediate edge representation for graph construction

Comparison with BGL
-------------------

The original BGL implementation requires:

- Explicit vertex iterator types
- Property maps for colors and discovery/finish times
- Visitor objects with multiple callbacks

NWGraph simplifies this with range-based for loops:

.. code-block:: cpp

   // BGL style (verbose)
   boost::depth_first_search(G, visitor(my_visitor));

   // NWGraph style (concise)
   for (auto&& v : dfs_range(A, 0)) {
       process(v);
   }

See Also
--------

- :doc:`ch4_loop_detection` - Uses DFS for cycle detection
- ``nwgraph/adaptors/dfs_range.hpp`` - DFS range adaptor implementation
