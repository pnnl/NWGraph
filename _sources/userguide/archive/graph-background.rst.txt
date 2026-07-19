.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:graph-background`:

Graph Background
================

In order to describe the NWGraph library and its abstractions and
interfaces, it is important to be precise about what we mean when we say
“graph,” not only in terms of the mathematical abstraction, but also in
terms of the corresponding software abstractions. To define our
terminology and to amplify our point regarding “graphs are not for
storing data,” we walk through the steps involved in representing the
implicit relationships in data tables as a graph.

**Remark.** Understanding graphs is necessary to develop requirements
for algorithms. However, it should be noted that we don’t derive those
requirements from the graph model, but instead from the algorithms. This
is a key distinction between generic programming and, say, OO
requirements analysis.

.. figure:: circuit_graph.pdf
   :alt: [fig:graph-model-circuit] Electrical circuit modeled as a
   directed graph. The nodes in the circuit are modeled as vertices and
   the circuit elements are modeled as edges.
   :name: fig:graph-model-circuit

   [fig:graph-model-circuit] Electrical circuit modeled as a directed
   graph. The nodes in the circuit are modeled as vertices and the
   circuit elements are modeled as edges.

.. figure:: airport_graph.pdf
   :alt: [fig:graph-model-airports] Airport route table modeled as an
   undirected graph. Airports are modeled as vertices and routes between
   cities are modeled as (weighted) edges.
   :name: fig:graph-model-airports

   [fig:graph-model-airports] Airport route table modeled as an
   undirected graph. Airports are modeled as vertices and routes between
   cities are modeled as (weighted) edges.

.. _`sec:graph-terminology`:

Graph Terminology
-----------------

Abstractly, we define a *graph* :math:`G` as comprising two finite sets,
:math:`G =\{ V, E \}`, where the set :math:`V` is a set of entities of
interest, “vertices” or “nodes,” and :math:`E` is a set of pairs of
entities from :math:`V`, “edges” or “links.” Without loss of generality
we label the entities in :math:`V` as :math:`v_i` so that
:math:`V = \{ v_0, v_1, \ldots v_{n-1} \}`. The set of edges, also
labeled, can be constructed using the labeled entities from :math:`V` so
that :math:`E = \{ e_0, e_1, \ldots e_{m-1} \}`. The edges may be
ordered pairs, denoted as :math:`(v_i, v_j)`, which have equality
defined such that
:math:`(v_i,v_j) = (v_m,v_n) \leftrightarrow v_i = v_m  \wedge v_j = v_n`.
Or, the edges may be unordered sets, denoted as :math:`\{v_i, v_j\}`
which have equality defined as
:math:`(v_i,v_j) = (v_m,v_n) \leftrightarrow\left( v_i = v_m  \wedge v_j = v_n\right) \vee \left( v_i = v_n  \wedge v_j = v_m\right)`.
If a graph is defined with ordered edges we say the graph is *directed*;
if the graph is defined with unordered edges we say the graph is
*undirected*.

Graph Models
------------

Graphs are powerful abstractions because they allow us to reason about
the relationships between entities, irrespective of what the entities
actually are. But, when we use graph algorithms in practice, we are
using them to model some specific problem. Since one of the motivations
behind NWGraph is to support graph computing in the context of real
programs, we briefly describe the first part of the abstraction process
when modeling with graphs.

Fig. `1 <#fig:graph-model-circuit>`__ shows a model of an electrical
circuit as a directed graph, both schematically, as a circle and line
diagram, and mathematically, as the sets :math:`V` and :math:`E`.
Two-terminal circuit elements connect to each other at given circuit
nodes. We thus model circuit connection points as graph vertices, and
the connections between them as edges. In the case of circuits,
orientation of circuit elements matters and so we use directed edges in
the graph.

Fig. `2 <#fig:graph-model-airports>`__ similarly shows a model of an
airport route table as an undirected graph. We begin with a table of
airports and a table of distances in kilometers between pairs of
airports. We model this situation as a graph by identifying graph nodes
with airports and graph edges with pairs of cities that are given as
pairs in the distance table.

.. _`sec:representing-graphs`:

Representing Graphs
-------------------

To define algorithms on graphs and to be able to reason about those
algorithms, we need to define some representations for graphs (and
corresponding terminology)—–not much can be done computationally with
abstract sets of vertices and edges. Various characteristics of these
representations are what we use to express algorithms (still abstractly)
but when those algorithms are implemented as generic library functions,
those characteristics will in turn become the basis for the library’s
concepts.

.. container:: figure*

   .. image:: circuit_index_graph_with_vit.pdf
      :alt: image
      :width: 90.0%

.. container:: figure*

   .. image:: airport_index_graph_with_vit.pdf
      :alt: image
      :width: 90.0%

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
Figs. `[fig:circuit_index_to_adj] <#fig:circuit_index_to_adj>`__
and `[fig:airport_index_to_adj] <#fig:airport_index_to_adj>`__ show the
progression from an index graph to an index adjacency list (compare also
to Figs. `1 <#fig:graph-model-circuit>`__
and `2 <#fig:graph-model-airports>`__). Since edges are given in terms
of vertex names, in order to create a list of edge indices, we need to
translate from vertex name to vertex index. Accordingly,
Figs. `1 <#fig:graph-model-circuit>`__
and `2 <#fig:graph-model-airports>`__ also show the translation table
from vertex to index.

Of course, we don’t need an underlying graph to define what an index
graph itself is. We can say that a graph :math:`G = (V, E)` is an index
graph if its vertex set is a set of contiguous indices, i.e., with
:math:`V=[0,|V|-1)`. Since an index graph is just a graph, in cases
where the context is clear, we may refer to an index graph simply as a
graph. We note that an adjacency list can only be defined over an index
graph.

Finally, we can make the following precise definition: An *adjacency
list* of an index graph :math:`G=(V,E)` is an array :math:`Adj(G)` of
size :math:`|V|` (the array is indexed from :math:`0` to :math:`|V|-1`)
with the following properties:

-  :math:`Adj(G)` is a container of :math:`|V|` containers, one
   container for each vertex in :math:`V`, and

-  The container :math:`Adj(G)[u]` contains all vertices :math:`v` for
   which there is an edge :math:`(u,v)\in E`.

This structure, **an adjacency list of an index graph, or an index
adjacency list, is the fundamental structure used by almost all graph
algorithms.** and show the index graph and the adjacency list
representation of our airport and circuit examples.

**Remark (1):** Although the standard term for this kind of abstraction
is “adjacency list”, and although it is often drawn schematically with
linked lists as elements, it is not necessary that this abstraction be
implemented as an actual linked list. In fact, other representations are
significantly more efficient. **What is important is that the items that
are stored, vertex indices, can be used to index into the adjacency list
to obtain other lists of neighbors.**

**Remark (2):** The index adjacency list does not store edges per se,
rather it stores lists of reachable neighbors. Therefore, the index
adjacency list is neither inherently directed nor undirected. That is,
for a given vertex :math:`u`, the container :math:`Adj(G)[u]` contains
the vertex :math:`v` if the edge :math:`(u,v)` is contained in
:math:`E`. This means that for a directed graph with edge :math:`(u,v)`
in , :math:`E`, :math:`Adj(G)[u]` will contain :math:`v`. For an
undirected graph with edge :math:`(u,v)` is contained in :math:`E`,
:math:`Adj(G)[u]` will contain :math:`v` and :math:`Adj(G)[v]` will
contain :math:`u`. **Directedness of the original graph is thus made
manifest in the values stored in the index adjacency list.**

Compare, for instance, the adjacency lists in
Figs. `[fig:circuit_index_to_adj] <#fig:circuit_index_to_adj>`__
and `[fig:airport_index_to_adj] <#fig:airport_index_to_adj>`__. The
graphs have the same structure in the schematics and in the mathematical
notations (shown in Figs. `1 <#fig:graph-model-circuit>`__
and `2 <#fig:graph-model-airports>`__). However, when realized as
adjacency list, the adjacency list in
Fig. `2 <#fig:graph-model-airports>`__ is symmetrized. Every edge
:math:`\{u,v\}` in the graph is inserted twice into the adjacency list:
once as :math:`\{u,v\}` and once as :math:`\{v,u\}`.
