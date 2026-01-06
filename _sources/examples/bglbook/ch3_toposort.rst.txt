.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=========================================================
File Dependencies - Topological Sort (BGL Book Chapter 3)
=========================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

A common use of the graph abstraction is to represent dependencies. One common type of
dependency that we programmers deal with on a routine basis is that of compilation dependencies
between files in programs that we write. Information about these dependencies is
used by programs such as ``make``, or by IDEs such as Visual C++, to determine which files
must be recompiled to generate a new version of a program (or, in general, of some target)
after a change has been made to a source file.

Consider a graph that has a vertex for each source file, object file, and library that
is used in a program. An edge in the graph shows that a target depends on another
target in some way (such as a dependency due to inclusion of a header file in a source file, or
due to an object file being compiled from a source file).

Answers to many of the questions that arise in creating a build system such as ``make`` can
be formulated in terms of the dependency graph:

* If all of the targets need to be made, in what order should that be accomplished?
* Are there any cycles in the dependencies? A dependency cycle is an error, and an
  appropriate message should be emitted.
* How many steps are required to make all of the targets? How many steps are required
  to make all of the targets if independent targets are made simultaneously in parallel?

Algorithm Description
--------------------

The first question—specifying an order in which to build all of the targets—requires ensuring
that before building a given target, all the targets that it depends on are already built.
This is the problem of computing a **topological ordering**.

A topological ordering can be computed using a depth-first search (DFS). A DFS visits all of
the vertices in a graph by starting at any vertex and then choosing an edge to follow. At the
next vertex another edge is chosen to follow. This process continues until a dead end (a vertex
with no out-edges that lead to a vertex not already discovered) is reached. The algorithm then
backtracks to the last discovered vertex that is adjacent to a vertex that is not yet discovered.

In the context of topological sorting, we record each vertex as it is **finished** (when the DFS
backtracks from that vertex). The reverse of this finish order is the topological order.

NWGraph Implementation
---------------------

NWGraph provides modern C++20 implementations of DFS and topological sorting that differ
from the BGL approach by using range adaptors instead of visitors.

.. literalinclude:: ../../../../examples/bgl-book/ch3_toposort.cpp
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
   make ch3_toposort

Then run the example:

.. code-block:: bash

   ./examples/bgl-book/ch3_toposort

The program reads a makefile dependency graph from a file and outputs the compilation order
(files listed in order such that each file appears before any files that depend on it).

Sample Output
------------

.. code-block:: text

   Compilation order:
   dax.h
   yow.h
   zow.h
   boz.h
   zig.cpp
   zig.o
   zag.cpp
   zag.o
   libzigzag.a
   foo.cpp
   foo.o
   bar.cpp
   bar.o
   libfoobar.a
   killerapp

Key NWGraph Features Demonstrated
--------------------------------

- **Edge list construction**: Building graphs from edge pairs
- **DFS range adaptor**: Using ``dfs_range`` to traverse the graph
- **Topological sorting**: Computing a valid build order for dependencies
- **Modern C++ idioms**: Range-based for loops and structured bindings

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 20.4: Topological Sort** and **Chapter 20.3: Depth-First Search**
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 3

See Also
--------

- :doc:`ch4_loop_detection` - Detecting cycles in dependency graphs
- :doc:`index` - BGL Book examples overview
