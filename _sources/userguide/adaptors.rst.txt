.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:graph_range_adaptors`:

Graph Range Adaptors
====================

A key feature of the new C++ Ranges is the notion of *views*, which
allow for different ways to access data in a range without changing the
underlying range. Between a range and a range view sits a range adaptor,
which takes the original range and presents it to the user as a view
while hiding the underlying data manipulation details. We leverage range
adaptors to simplify graph algorithms in NWGraph, by providing reusable
data access patterns which eliminate the need for visitor objects.

For example, consider again BFS traversal, a core graph algorithm
kernel. Except perhaps for benchmarking, a standalone BFS traversal is
rarely useful. Rather, other algorithms use a BFS traversal pattern to
perform more useful computations, such as finding the distance to every
vertex from the source, finding the parent list, etc. One approach to
making BFS traversal to other types of computations would be to further
parameterize ``bfs`` with additional functions. However, what to apply
and where to apply it is not well defined – we don’t necessarily have
well-defined concrete algorithms to lift.

The Boost Graph Library provided extensibility to BFS through its
*Visitor* mechanism, which was essentially a large structure with
callbacks used at multiple select points in the BFS
execution :cite:`Siek_Lee_Lumsdaine_2002`. The BFS Visitor
has nine different possible callbacks, making actual extension of its
BFS a complicated proposition.

NWGraph takes a different approach. Rather than attempting to further
lift a given algorithm for arbitrary (and not well-defined) concrete use
cases, NWGraph provides range adaptors that allow the graph to be
iterated over (either vertex by vertex or edge by edge) in a specified
order. For example, NWGraph provides ``bfs_range`` for traversing the
vertices of a graph in breadth-first order, and ``bfs_edge_range`` for
travering the edges.

.. code:: c++

     for (auto&& u : bfs_range(G)) { 
       /* visit vertex u */ }
     for (auto&& [u, v] : bfs_edge_range(G)) {
       /* Visit edge u,v */ }

As views are concise and efficient ways of representing the same data in
multiple ways, graph algorithms can be considered as operating on a
range of elements of a graph with different requirements on how data is
being viewed by the algorithm. In NWGraph, we provide three categories
of view of the graph shown in :

-  *Original view of the graph:* These include edge range, neighbor
   range, plain range, random range and back-edge range.

-  *Modified view of the graph based on traversal criteria:* For
   example, BFS and DFS traversal-based algorithms consider vertices in
   a certain order. These alternative views include BFS edge range,
   filtered BFS range, DFS edge range, Directed Acyclic Graph (DAG)
   range and Reverse Path. More sophisticated range adaptor such as DAG
   range, for example, iterates over the vertices in a particular order,
   based on the predecessor-successor relationships, imposed by a
   heuristic (degree-based, largest-degree first, smallest-degree last
   etc.), and is needed for graph coloring algorithms.

-  *Subview of the graph for workload distribution in parallel
   execution:* These include splittable edge range and cyclic range.

.. figure:: graphics/graph_adoptors_updated-crop.pdf
   :alt: Range adaptors in NWGraph.
   :name: fig:range_adaptors

   Range adaptors in NWGraph.
