.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 9: Implicit Graphs - A Knight's Tour
============================================

Overview
--------

This example demonstrates solving the knight's tour problem using an
implicit graph representation. The knight's tour is finding a path for
a chess knight to visit every square on an n×n chessboard exactly once.

**Key concepts demonstrated:**

- **Implicit graphs**: Graph structure computed on-the-fly rather than stored
- **Backtracking search**: Systematic exploration with undo capability
- **Warnsdorff's heuristic**: Greedy choice to improve performance dramatically

The knight's tour is an example of a Hamiltonian path problem, which is
NP-complete in general. However, for chessboards, heuristics make it tractable.

Algorithm Description
--------------------

**Knight's Movement:**

A knight moves in an "L" pattern: two squares in one direction and one
square perpendicular, or vice versa. From any square, a knight has at most
8 possible moves.

.. code-block:: text

      . X . X .
      X . . . X
      . . N . .
      X . . . X
      . X . X .

**Backtracking Algorithm:**

1. Start from initial position
2. Mark current square as visited
3. Try each possible knight move:
   a. If move leads to unvisited square, recurse
   b. If recursion succeeds, return success
   c. If recursion fails, try next move
4. If all moves fail, backtrack (mark square unvisited)

**Time complexity**: O(8^(n²)) worst case - exponential!

**Warnsdorff's Heuristic:**

Instead of trying moves in arbitrary order, choose the move that leads to
the square with the **fewest onward moves**. This greedy choice:

- Visits constrained corners/edges first
- Avoids getting trapped with no valid moves
- Reduces complexity to approximately O(n²) in practice

NWGraph Implementation
---------------------

The implicit graph is represented as a class that computes neighbors on demand:

.. literalinclude:: ../../../../examples/bgl-book/ch9_knights_tour.cpp
   :language: cpp
   :linenos:
   :lines: 1-50
   :caption: File header and Position struct

The implicit graph class:

.. literalinclude:: ../../../../examples/bgl-book/ch9_knights_tour.cpp
   :language: cpp
   :linenos:
   :lines: 61-100
   :caption: KnightsTourGraph - implicit graph representation

Warnsdorff's heuristic implementation:

.. literalinclude:: ../../../../examples/bgl-book/ch9_knights_tour.cpp
   :language: cpp
   :linenos:
   :lines: 174-213
   :caption: Warnsdorff's heuristic

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch9_knights_tour

Sample Output
------------

.. code-block:: text

   === Knight's Tour: Implicit Graphs ===
   Based on BGL Book Chapter 9

   The knight's tour problem: find a path for a knight
   to visit every square on an n×n chessboard exactly once.

   Knight moves in an 'L' pattern:
     (±2, ±1) or (±1, ±2)

   === Small Board (5×5) with Warnsdorff ===
   Starting from position (0, 0)...
   Note: Pure backtracking is O(8^25) for 5×5 - impractical!
   Using Warnsdorff's heuristic instead...
   Found a knight's tour!

       0  1  2  3  4
   0   0  9 18 13  4
   1  17 12  3  8 19
   2   2  7 10 19 14
   3  11 16 23 20  5
   4   6  1 22 15 24

   === Standard Board (8×8) with Warnsdorff's Heuristic ===
   Starting from position (0, 0)...
   Found a knight's tour!

       0  1  2  3  4  5  6  7
   0   0 11 16 21 58 13 18 63
   1  15 20 57 12 17 62 59 14
   2  10  1 22 61 56 19 64 53
   ...

Key NWGraph Features Demonstrated
--------------------------------

- **Implicit graph pattern**: No explicit edge storage
- **On-demand neighbor computation**: ``adjacent_vertices()`` method
- **Position-to-index mapping**: Linearizing 2D coordinates
- **Algorithm flexibility**: Same patterns work for implicit and explicit graphs

Implicit Graph Interface
------------------------

An implicit graph provides the same interface as an explicit graph:

.. code-block:: cpp

   class ImplicitGraph {
   public:
       // Number of vertices
       size_t num_vertices() const;

       // Get neighbors of a vertex (computed on-the-fly)
       std::vector<size_t> adjacent_vertices(size_t v) const;

       // Optional: size for iteration
       size_t size() const { return num_vertices(); }
   };

This pattern is useful when:

- Graph is too large to store explicitly
- Edge structure follows a simple rule (grid, knight moves, etc.)
- Only local structure is needed (no global queries)

Warnsdorff's Heuristic Details
------------------------------

The heuristic counts future moves for each option:

.. code-block:: cpp

   // Count unvisited neighbors of a position
   int count_onward_moves(Position pos) {
       int count = 0;
       for (auto&& next : graph.adjacent_vertices(pos)) {
           if (!visited[next]) count++;
       }
       return count;
   }

   // Choose move with minimum onward moves
   Position choose_next(Position current) {
       Position best;
       int min_count = 9;  // More than max (8)

       for (auto&& next : graph.adjacent_vertices(current)) {
           if (!visited[next]) {
               int count = count_onward_moves(next);
               if (count < min_count) {
                   min_count = count;
                   best = next;
               }
           }
       }
       return best;
   }

**Why it works:** Squares in corners and edges have fewer moves available.
By visiting them first, we ensure they don't become unreachable dead ends.

Closed vs Open Tours
--------------------

- **Open tour**: Knight ends on any square (what this example finds)
- **Closed tour**: Knight ends one move from the starting square (Hamiltonian cycle)

Closed tours are harder to find but exist for all boards n ≥ 5.

See Also
--------

- :doc:`ch4_loop_detection` - DFS and backtracking
- :doc:`ch3_toposort` - DFS traversal
- Graph concepts documentation for implicit graph patterns
