.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=================================================
Six Degrees of Kevin Bacon (BGL Book Chapter 4.1)
=================================================

*Based on "The Boost Graph Library" by Jeremy Siek, Lie-Quan Lee, and Andrew Lumsdaine*

Overview
--------

An amusing application of breadth-first search comes up in the popular game "Six Degrees
of Kevin Bacon." The idea of the game is to connect an actor to Kevin Bacon through a trail
of actors who appeared together in movies, and do so in less than six steps.

For example, Theodore Hesburgh (President Emeritus of the University of Notre Dame) was
in the movie *Rudy* with the actor Gerry Becker, who was in the movie *Sleepers* with
Kevin Bacon. Why Kevin Bacon? For some reason, the three students who invented the game—
Mike Ginelli, Craig Fass, and Brian Turtle—decided that Kevin Bacon was the center of the
entertainment world.

Mathematicians play a similar game; they keep track of their *Erdős number*, which is
the number of co-authored publications that separate them from the famous Paul Erdős.

The Graph Model
---------------

The "Six Degrees of Kevin Bacon" game is really a graph problem. The graph representing
the problem can be modeled by:

* Assigning a **vertex** for each actor
* Creating an **edge** between two vertices if the corresponding actors have appeared
  together in a movie

Since the relationship between actors appearing together in a movie is symmetric, edges
between actors can be undirected, resulting in an **undirected graph**.

Algorithm Description
--------------------

The problem of finding a trail of actors to Kevin Bacon becomes a traditional graph
problem—that of finding a path between two vertices. Since we wish to find a path that
is shorter than six steps, ideally we would like to find the **shortest path** between vertices.

Breadth-first search (BFS) can be used to find shortest paths in unweighted graphs.
Similar to the Erdős number, we use the term **Bacon number** to mean the shortest path
length from a given actor to Kevin Bacon.

BFS works by exploring vertices in order of their distance from a source vertex:

1. Start at the source vertex (Kevin Bacon)
2. Visit all vertices one edge away
3. Then visit all vertices two edges away
4. Continue until all reachable vertices are visited

The distance at which each vertex is discovered is its shortest distance from the source.

NWGraph Implementation
---------------------

NWGraph provides a ``bfs_range`` adaptor that yields vertices in BFS order, making it
easy to compute shortest path distances.

.. literalinclude:: ../../../../examples/bgl-book/ch4_kevin_bacon.cpp
   :language: cpp
   :linenos:
   :caption: Complete source code

Running the Example
------------------

Build and run the example:

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch4_kevin_bacon kevin-bacon.dat

The program reads an actor-movie database and computes the Bacon number for each actor.

Sample Output
------------

.. code-block:: text

   Kevin Bacon has a Bacon number of 0
   Patrick Stewart has a Bacon number of 1
   Steve Martin has a Bacon number of 1
   Glenn Close has a Bacon number of 1
   Tom Hanks has a Bacon number of 1
   ...

Key NWGraph Features Demonstrated
--------------------------------

- **Undirected graphs**: Using symmetric edges for actor relationships
- **BFS traversal**: Using ``bfs_range`` to compute shortest paths
- **Property maps**: Associating actor names with vertices
- **Unweighted shortest paths**: Computing Bacon numbers via BFS distances

References
----------

- Cormen, Leiserson, Rivest, and Stein. *Introduction to Algorithms*, 4th Edition (2022),
  **Chapter 20.2: Breadth-First Search**
- Siek, Lee, and Lumsdaine. *The Boost Graph Library* (2002), Chapter 4.1

See Also
--------

- :doc:`ch4_loop_detection` - Using DFS to detect cycles
- :doc:`ch5_dijkstra` - Weighted shortest paths
- :doc:`index` - BGL Book examples overview
