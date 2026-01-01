.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 4.1: Six Degrees of Kevin Bacon
============================================

Overview
--------

This example demonstrates breadth-first search (BFS) to compute the "Bacon
number" - the number of connections from any actor to Kevin Bacon through
co-starring in movies. This is a popular illustration of the "small world"
phenomenon, where most people are connected by surprisingly short chains.

The problem involves finding shortest paths in an unweighted graph, which is
exactly what BFS provides. Each edge represents two actors appearing in the
same movie, and the Bacon number is the number of edges in the shortest
path to Kevin Bacon.

Algorithm Description
--------------------

BFS explores vertices in order of their distance from the source. Starting
from Kevin Bacon (distance 0), we first visit all actors who co-starred
with him (distance 1), then all actors who co-starred with those actors
(distance 2), and so on.

**Algorithm properties:**

- Visits vertices in increasing order of distance from source
- Finds shortest paths in unweighted graphs
- Time complexity: O(V + E)
- Space complexity: O(V) for the queue and distance array

**Data transformation:**

The raw data is a bipartite graph: actors and movies with edges between
them. Two actors are connected if there's a length-2 path through a movie.
We transform this to a direct actor-actor graph.

NWGraph Implementation
---------------------

The example demonstrates NWGraph's "there is no graph" philosophy by
starting from raw database records and building the graph on-the-fly:

.. literalinclude:: ../../../../examples/bgl-book/ch4_kevin_bacon.cpp
   :language: cpp
   :linenos:
   :lines: 1-30
   :caption: File header and includes

The data is read from a semicolon-separated file with actor-movie-actor
records:

.. literalinclude:: ../../../../examples/bgl-book/ch4_kevin_bacon.cpp
   :language: cpp
   :linenos:
   :lines: 32-65
   :caption: Parsing IMDB data and building the graph

The BFS uses NWGraph's ``bfs_edge_range`` adaptor, which yields edges in
BFS order:

.. literalinclude:: ../../../../examples/bgl-book/ch4_kevin_bacon.cpp
   :language: cpp
   :linenos:
   :lines: 67-99
   :caption: Computing Bacon numbers with BFS

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch4_kevin_bacon kevin-bacon.dat

Where ``kevin-bacon.dat`` contains actor-movie-actor records:

.. code-block:: text

   Kevin Bacon;Footloose (1984);Lori Singer
   Lori Singer;The Falcon and the Snowman;David Suchet
   ...

Sample Output
------------

.. code-block:: text

   === Six Degrees of Kevin Bacon ===
   Based on BGL Book Chapter 4.1

   Bacon numbers:
     Kevin Bacon has Bacon number of 0
     Lori Singer has Bacon number of 1
     David Suchet has Bacon number of 2
     ...

Key NWGraph Features Demonstrated
--------------------------------

- **bfs_edge_range**: Lazy BFS traversal yielding (source, target, edge_data)
- **edge_list**: Building graphs from raw data with push_back
- **adjacency<0>**: Forward adjacency list for BFS traversal
- **Structured bindings**: C++17/20 syntax for tuple unpacking

Graph Construction Pattern
-------------------------

This example shows a common NWGraph pattern: building a graph from
relational data:

.. code-block:: cpp

   // 1. Create maps to assign IDs
   std::map<std::string, size_t> name_to_id;

   // 2. Build edge list while assigning IDs
   edge_list<directedness::undirected, std::string> edges;
   for (auto&& record : data) {
       auto id1 = get_or_create_id(record.actor1);
       auto id2 = get_or_create_id(record.actor2);
       edges.push_back(id1, id2, record.movie);
   }

   // 3. Convert to adjacency for efficient traversal
   adjacency<0, std::string> G(edges);

See Also
--------

- :doc:`../degrees/index` - More detailed Kevin Bacon example
- :doc:`ch5_dijkstra` - Shortest paths with weighted edges
- ``nwgraph/adaptors/bfs_edge_range.hpp`` - BFS edge range implementation
