.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:graph-background`:

Graph Background
================

We define a *graph* :math:`G` as comprising two finite sets,
:math:`G =\{ V, E \}`, where the set
:math:`V= \{ v_0, v_1, \ldots v_{n-1} \}` is a set of entities of
interest, “vertices” or “nodes,” and
:math:`E = \{ e_0, e_1, \ldots e_{m-1} \}` is a set of pairs of entities
from :math:`V`, “edges” or “links.” Edges may be ordered or unordered; a
graph defined with ordered edges is said to be *directed*; a graph
defined with unordered edges is said to be *undirected*.

**Remark.** Understanding graphs is necessary to develop requirements
for algorithms. However, it should be noted that we don’t derive those
requirements from the graph model, but *instead from the algorithms*.
This is a key distinction between generic programming and, say,
Object-oriented (OO) requirements analysis.

.. _`sec:representing-graphs`:

Representing Graphs
-------------------

To define algorithms on graphs and to be able to reason about those
algorithms, we need to define some representations for graphs; not much
can be done computationally with abstract sets of vertices and edges.
The specific characteristics of these representations are what we use to
express algorithms (still abstractly) but when those algorithms are
implemented as generic library functions, those characteristics will in
turn become the basis for the library’s interfaces (represented in our
case as C++ concepts).

One of the fundamental operations in graph algorithms is a *traversal*.
That is, given a vertex :math:`u`, we would like to find the *neighbors*
of :math:`u`, i.e., all vertices :math:`v` such that the edge
:math:`(u,v)` is in the graph. Then, for each of those edges, we would
like to find their neighbors, and so on. The representation that we can
define to make this efficient is an *adjacency list*.

Given a graph :math:`G = (V,E)`, we can define an adjacency-list
representation in the following way. Assign to each element of :math:`V`
a unique index from the range :math:`[0,|V|)` and denote the vertex
identified with index :math:`i` as :math:`V[i]`. We can now define a new
graph with the same structure as :math:`G`, but in terms of the indices
in :math:`[0,|V|)`, rather than with the elements in :math:`V`. Let the
*index graph of* :math:`G` be the graph :math:`G'=(V',E')`, where
:math:`V'=[0,|V|)` and :math:`E'` consists of :math:`|E|` pairs of
indices from :math:`V`, such that a pair :math:`(i,j)` is in E’ if and
only if :math:`(V[i],V[j])` is in :math:`E`. Which is all to say, the
index graph of :math:`G` is the graph we get by replacing all elements
of :math:`G` with their corresponding indices.

We make the following precise definition: An *adjacency list* of an
index graph :math:`G=(V,E)` is an array :math:`Adj(G)` of size
:math:`|V|` (the array is indexed from :math:`0` to :math:`|V|-1`) with
the following properties:

-  :math:`Adj(G)` is a container of :math:`|V|` containers, one
   container for each vertex in :math:`V`, and

-  The container :math:`Adj(G)[u]` contains all vertices :math:`v` for
   which there is an edge :math:`(u,v)\in E`.

This structure, an adjacency list of an index graph, or an index
adjacency list, is the fundamental structure used by almost all graph
algorithms.

**Remark:** Although the standard term for this kind of abstraction is
“adjacency list”, and although it is often drawn schematically with
linked lists as elements, it is not necessary that this abstraction be
implemented as an actual linked list. In fact, other representations
(such as compressed sparse row storage) are significantly more
efficient. What is important is that the items that are stored, vertex
indices, can be used to index into the adjacency list to obtain other
lists of neighbors.

