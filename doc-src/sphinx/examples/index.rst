.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

========
Examples
========

This section presents examples demonstrating how to use NWGraph for various
graph algorithms and applications. The examples are organized to help you
understand both the library's capabilities and the underlying algorithms.

.. toctree::
   :maxdepth: 2
   :caption: Example Categories

   bglbook/index
   degrees/index
   imdb/index

BGL Book Examples
-----------------

Examples adapted from "The Boost Graph Library" book by Jeremy G. Siek,
Lie-Quan Lee, and Andrew Lumsdaine (Addison-Wesley, 2002). These demonstrate
how NWGraph provides similar functionality using modern C++20 idioms.

- **Chapter 3**: Topological Sort - File dependencies
- **Chapter 4**: BFS and DFS - Kevin Bacon numbers, loop detection
- **Chapter 5**: Shortest Paths - Dijkstra and Bellman-Ford
- **Chapter 6**: Minimum Spanning Trees - Kruskal and Prim
- **Chapter 7**: Connected Components - BFS-based and Tarjan's SCC
- **Chapter 8**: Maximum Flow - Edmonds-Karp algorithm
- **Chapter 9**: Implicit Graphs - Knight's Tour

Six Degrees of Separation
-------------------------

A detailed walkthrough showing how to compute "Bacon numbers" using NWGraph,
demonstrating the "there is no graph" design philosophy by starting from
raw database records and building graph structures on-the-fly.

IMDB Examples
-------------

Real-world examples using Internet Movie Database data to demonstrate
graph analysis at scale, including the Oracle of Bacon implementation.
