.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=====================================================
Implicit Graphs: A Knight's Tour (BGL Book Chapter 9)
=====================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

The **knight's tour problem** is as follows: Find a path for a knight to touch all of
the squares of an n x n chessboard exactly once.

The knight's tour is an example of a **Hamiltonian path**—a simple path that passes
through each vertex of the graph exactly once. Each square of the chessboard is treated
as a vertex in the graph, and edges are determined by the pattern in which a knight can
jump (up two and over one, or vice versa).

The Hamiltonian path problem is **NP-complete**, meaning that for large problem sizes it
cannot be solved in a reasonable amount of time using brute-force methods.

Implicit Graph Representation
-----------------------------

One unique aspect of this example is that it does not use an explicit data structure
(such as adjacency list) to represent the graph. Rather, an **implicit graph structure**
is deduced from the allowable moves of a knight on a chessboard.

The knight's possible jumps are stored in an array::

   Position knight_jumps[8] = {
     {2, -1}, {1, -2}, {-1, -2}, {-2, -1},
     {-2, 1}, {-1, 2}, {1, 2}, {2, 1}
   };

The adjacency iterator iterates through these possible jumps, filtering out moves that
would go off the board.

Backtracking Graph Search
-------------------------

The basic algorithm is a **backtracking graph search**, similar to depth-first search.
The idea is:

1. Explore a path until a dead end is reached
2. After a dead end, back up (unmarking the path) before continuing
3. Try a different path
4. Repeat until all vertices are visited or all paths exhausted

The search uses a stack containing timestamp-vertex pairs, so the proper timestamp is
available after backtracking from a dead end.

Warnsdorff's Heuristic
----------------------

The basic backtracking search is very slow (brute force). **Warnsdorff's heuristic**
dramatically improves performance by choosing the next move intelligently:

  *Always move to the square with the fewest onward moves.*

This greedy strategy tends to avoid getting trapped in corners early, leaving those
difficult squares for later when they can be reached more easily. With this heuristic,
the knight's tour can be solved efficiently even for larger boards.

NWGraph Implementation
---------------------

NWGraph implements the knight's tour using an implicit graph that models the
AdjacencyGraph concept, demonstrating that graph algorithms work with any data
structure that provides the required interface.

.. literalinclude:: ../../../../examples/bgl-book/ch9_knights_tour.cpp
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
   make ch9_knights_tour

Then run the example:

.. code-block:: bash

   ./examples/bgl-book/ch9_knights_tour

The program finds a knight's tour on an 8x8 chessboard (or reports if none exists for
smaller boards).

Sample Output
------------

.. code-block:: text

   Knight's Tour (8x8 board)

   Using Warnsdorff's heuristic...

    0  59  38  33  30  17   8  63
   37  34  31  60   9  62  29  16
   58   1  36  39  32  27  18   7
   35  48  41  26  61  10  15  28
   42  57   2  49  40  23   6  19
   47  50  45  54  25  20  11  14
   56  43  52   3  22  13  24   5
   51  46  55  44  53   4  21  12

   Tour found in 0.003 seconds

Key NWGraph Features Demonstrated
--------------------------------

- **Implicit graphs**: No explicit adjacency list storage
- **Custom graph types**: Implementing required concept interfaces
- **Backtracking search**: DFS with undo capability
- **Heuristic optimization**: Warnsdorff's rule for efficiency
- **Hamiltonian paths**: Classic combinatorial problem

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 34: NP-Completeness** (Hamiltonian path is discussed as an NP-complete problem)
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 9

See Also
--------

- :doc:`ch4_loop_detection` - DFS-based algorithms
- :doc:`ch3_toposort` - Another DFS application
- :doc:`index` - BGL Book examples overview
