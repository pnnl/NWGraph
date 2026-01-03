.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:concepts`:

Generic Graph Algorithms
========================

In this section we analyze the requirements for graph algorithms in
order to derive generic graph algorithms. In NWGraph, these generic
algorithms are realized as function templates, and the type requirements
are realized as C++20 ``concept``\ s. Our process centers on defining
type requirements at the interfaces to algorithms based on what the
algorithms actually need from their types, rather than starting with
graph types and building algorithms to those types.

.. _`sec:algorithm_requirements`:

Algorithm Requirements
----------------------

Algorithms in the STL operate over containers. The concepts defined for
the STL have to do with mechanisms for traversing a container and
accessing the data therein. Since graphs in some sense are also
containers of data, we can reuse the mechanisms from the STL for
traversing graphs and accessing graph data, to the extent that makes
sense. However, graphs are *structured data* and graph algorithms
traverse that structure in various ways. Accordingly, our graph concepts
must support structured traversal of graphs.

Most (but not all) graph algorithms traverse a graph vertex to vertex by
following the edges that connect vertices. For implementing such
algorithms, it is assumed that a graph :math:`G = \{V, E\}` is
represented with an adjacency-list structure  [1]_ as defined in .

.. _`sec:requirements_concrete_alg`:

Requirements for Concrete Algorithms
------------------------------------

A prototypical algorithm in this class is the breadth-first search (BFS)
algorithm. The pseudocode for this algorithm, along with its C++
implementation is shown in Fig. `[fig:bfs] <#fig:bfs>`__. The algorithm
is abbreviated from:cite:`cormen2009introduction`. Modulo
some type declarations that would be necessary for real code to compile,
but which can be omitted from pseudocode, the C++ code, using out of the
box language mechanisms and library components, has essentially a
one-one correspondence to the pseudocode.

.. container:: figure*

   --------------

   .. container:: codebox

      :math:`\displaystyle color[u] \gets \const{\textsc{white}}`
      :math:`\displaystyle u\gets\proc{Dequeue}{(Q)}`
      :math:`\displaystyle  color[v] \gets \const{gray}`

   --------------

   .. code:: cpp

      void bfs(const Graph& G, int s) { ...
        for (int u = 0; u < size(G); ++u)
          color[u] = WHITE;
        color[s] = GREY;
        std::queue<int> Q;
        Q.push(s);
        while (!Q.empty()) {
          auto u = Q.front();    Q.pop();
          for (auto&& v : G[u]) { 
            if (color[v] == WHITE) {
              color[v] == GREY;
              Q.push(v);            }}
          color[u] = BLACK;         }}

From this implementation we can extract an initial set of requirements
for the BFS algorithm:

-  The graph ``G`` is a *random access range*, meaning it can be indexed
   into with an object (of its difference type) and it has a size.

-  The value type of ``G`` (the inner range of ``G``) is a *forward
   range*, meaning it is something that can be iterated over and have
   values extracted.

-  The value type of the inner range is something that can be used to
   index into ``G``.

-  All elements stored in ``G`` must be able to correctly index into it,
   meaning their value are between 0 and ``size(G)-1``, inclusive.

Associated with the concepts of a random access range and forward range
are complexity guarantees (which are also implied by the theoretical
algorithm). Indexing into ``G`` is a constant-time operation and
iterating over the elements in ``G[u]`` is linear in the number of
elements stored in ``G[u]``.

As an example, we could use any of the following compositions of
standard library components for the ``Graph`` datatype above:

.. code:: cpp

   using Graph = std::vector<std::list<int>>;
   using Graph = std::vector<std::vector<unsigned>>;
   using Graph = std::vector<std::forward_list<size_t>>;  

In fact, any ``Graph`` data structure meeting the above requirements
could be used.

We now have a set of requirements for a concrete implementation of BFS.
Following the generic programming process, there are various aspects of
the implementation that we could begin lifting. Ultimately, as with the
STL, we want a set of concepts useful across families of graph
algorithms. So rather than lifting BFS in isolation, we now examine
concrete implementations of other algorithms in order to identify common
functionality that can be lifted in order to unify abstractions.

.. container:: figure*

   --------------

   .. container:: codebox

      :math:`\displaystyle d[u] \gets \infty`
      :math:`\displaystyle u\gets\proc{Extract-Min}{(Q)}`
      :math:`d[v] \gets d[u] + w(u,v)`

   --------------

   .. code:: cpp

      void dijkstra(const Graph& G, int s) {...
        for (int u = 0; u < size(G); ++u) {
          d[u] = INF;
          pi[u] = NIL;                    }
        d[s] = 0
        for (int u = 0; u < size(G); ++u)   
          Q.push({u, d[u]});
        while (!Q.empty()) {
          auto [u, x] = Q.top(); Q.pop();
          for (auto&& [v, w] : G[u]) {
            if (d[u] + w < d[v]) {
              d[v] = d[u] + w;
              pi[v] = u;               }}}}

Fig. `[fig:dijkstra] <#fig:dijkstra>`__ shows the pseudocode and
corresponding C++ implementation for Dijksra’s algorithm for solving the
single-source shortest paths problem. From this implementation we can
extract an initial set of requirements for the concrete ``dijkstra``
algorithm:

-  The graph ``G`` is a *random access range*.

-  The value type of ``G`` (the inner range of ``G``) is a *forward
   range*.

-  The value type of the inner range is a pair, consisting of a
   something we will call a vertex type and something we will call a
   weight.

-  The vertex type is something that can be used to index into ``G``.

-  All values stored as vertex types in ``G`` must be able to correctly
   index into ``G`` meaning their value are between 0 and ``size(G)-1``,
   inclusive.

Just as the code of ``dijkstra`` is similar to ``bfs``, some of these
requirements are also the same. **However, the key difference is in what
is stored inside of the graph**. This implementation of ``dijkstra``
assumes that the graph stores a tuple consisting of *a vertex value and
an edge weight*. That is, rather than the ``Graph`` types shown above,
we could use the following for ``dijkstra``:

.. code:: cpp

   using Graph = std::vector<std::list<std::tuple<size_t, int>>>;
   using Graph = std::vector<std::vector<std::tuple<unsigned, double>>>;
   using Graph = std::vector<std::forward_list<std::tuple<int, float>>>;

This is a different kind of graph than we had for ``bfs``, which only
stored a value. Yet, even a graph that stores a weight on its edge is
suitable for BFS exploration. Similarly, a graph without a weight on its
edge should be suitable for Dijkstra’s algorithm, provided a weight
value can be provided in some way, or a default value, say, 1, used.

.. _`subsec:lifting_graph`:

Lifting
-------

From the foregoing discussion, we have two pieces of functionality we
need to lift. First, we need to lift how the neighbor vertex is stored
so that whether it is stored as a direct value or as part of a tuple (or
any other way), it can be obtained. Second, we need to lift how weights
(or, more generally, **properties**) are stored on edges. And, finally,
implied when we say we want to use different kinds of graphs with these
algorithms, we need to parameterize them on the graph type (make them
function templates rather than functions).

Parameterizing the Graph Type
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this lifting process we will be building up to a concept, which we
will illustrate by lifting ``dijkstra``. We begin by presenting its type
parameterization. The prototype for a ``dijkstra`` function template
based on our previous definition would be

.. code:: cpp

   template <class Graph>
   auto dijkstra(const Graph& G, vertex_id_t<G> s);

Note that we have parameterized *two* things: the ``Graph`` type itself,
as well as the type of the starting vertex ``s``. In this case, the
vertex type is not arbitrary, it is related to the type of the graph,
and so we have a type primitive ``vertex_id_t`` that returns the type of
the vertex associated with graph ``G``.

We can update some of the previous requirements for the
type-parameterized ``dijkstra``:

-  ``Graph`` must meet the requirements of ``random_access_range``.

-  The value type of ``Graph`` (the inner range of ``Graph``) must meet
   the requirements of ``forward_range``.

-  The type ``vertex_id_t<Graph>`` is an associated type of ``Graph``.

-  The type ``vertex_id_t<Graph>`` is convertible to the
   ``range_difference_t`` of ``Graph`` (that is, it can be used to index
   into a ``Graph``).

Both classes of graphs that we had previously seen for ``bfs`` and
``dijkstra`` satisfy these requirements (which are more general than
either of the previous requirements). For example, both of the following
compound structures

.. code:: cpp

   std::vector<std::vector<int>>;
   std::vector<std::vector<std::tuple<int, float, double>>>;

satisfy the lifted requirements, (provided a suitable overload of
``vertex_id_t`` is defined) though each would have only satisfied one of
the previous requirements. Note however, we still need to do more
lifting before we can actually compose ``bfs`` or ``dijkstra`` with
these types.

Lifting Neighbor Access
~~~~~~~~~~~~~~~~~~~~~~~

How a neighbor is stored is dependent on the graph structure itself. We
want the mechanism for accessing it therefore to vary based on the graph
type. In keeping with standard C++ practice – and since we want to be
able to use with C++ standard library containers, we adopt a polymorphic
free function interface to abstract the process of accessing a neighbor.
In particular, we define a ``target`` *customization point object* (CPO)
to abstract how a neighbor vertex is accessed, given an object obtained
from traversing the neighbor list.

-  If variable ``G`` is of type ``Graph`` and variable ``e`` is of the
   value type of the inner range of ``Graph``, then ``target(G, e)`` is
   a valid expression that returns a type of ``vertex_id_t<Graph>``.

-  All values returned by ``target(G, e)`` must be able to correctly
   index into a ``Graph`` ``G``.

With this abstraction, the loop and neighbor access in ``bfs`` and
``dijkstra`` (respectively at
lines `[bfs:foreach_v_in_Adj] <#bfs:foreach_v_in_Adj>`__
and `[code:foreach-dijkstra] <#code:foreach-dijkstra>`__) are replaced
by

.. code:: cpp

   for (auto&& e : G[u]) {
     auto v = target(G, e);
     ... }

Now, provided that suitable overloads for ``target`` are defined, the
two model graph types

.. code:: cpp

   std::vector<std::vector<int>>;
   std::vector<std::vector<std::tuple<int, float, double>>>;

will satisfy the above requirements, and we can compose them with the
``bfs`` and ``dijkstra`` we have lifted to this point.

We can, for example, define overloads for target thusly:

.. code:: cpp

   int target(const std::vector<std::vector<int>>& G, int e) { return e; }
   using E = std::tuple<int, float, double>;
   int target(const std::vector<std::vector<E>>& G, E& e) { return std::get<0>(e); }

Note that these overloads are each specific to a single graph type. In
practice we can define generalized overloads for entire classes of
containers. In NWGraph we opted to realize ``target`` as a CPO,
implemented using the ``tag_invoke``
mechanism:cite:`_tag_invoke`.


Graph Concepts: Encapsulating Lifted Requirements
-------------------------------------------------

We can encapsulate (and formalize) the above requirements in the form of
a concept (which is almost a direct translation of the stated
requirements to code).

We first capture the very fundamental requirements of a graph, that it
has an associated vertex id type:

.. code:: cpp

   template <typename G>
   concept graph = std::semiregular<G> 
       && requires(G g) { typename vertex_id_t<G>;  };

We define this as a separate concept since we may wish to define other
concepts that reuse these requirements.

Next, we define some convenience type aliases to capture the type of the
inner range of a graph as well as the type that is stored by the inner
range:

.. code:: cpp

   template <typename G>
   using inner_range = std::ranges::range_value_t<G>;
   template <typename G>
   using inner_value = std::ranges::range_value_t<inner_range<G>>;

Now we can define the concept that captures the requirements from the
lifted ``bfs`` and ``dijkstra``:

.. code:: cpp

   template <typename G>
   concept adjacency_list = graph<G>
       && std::ranges::random_access_range<G>
       && std::ranges::forward_range<inner_range_t<G>>
       && std::convertible_to<vertex_id_t<G>, std::ranges::range_difference_t<G>>
       && requires(G g, vertex_id_t<G> u, inner_value_t<G> e) {
     { g[u] } -> std::convertible_to<inner_range_t<G>>;
     { target(g, e) } -> std::convertible_to<vertex_id_t<G>>; };

Although we restricted our illustration of lifting to ``bfs`` and
``dijkstra`` in this paper, this concept applies to almost all of the
algorithms in NWGraph (and, since it represents an adjacency list, we
expect it could apply to the majority of, if not all, graph algorithms
based on adjacency lists).

We can use this concept to constrain the interface to ``bfs`` in the
following two ways:

.. code:: cpp

   template <class Graph>
   requires adjacency_list<Graph>   
   void bfs(const Graph& G);  

.. code:: cpp

   template <adjacency_list Graph>
   void bfs(const Graph& G);  

Generally there is a fully general declaration when using concepts via
the ``requires`` keyword, and a number of abbreviated forms. In the
NWGraph library, the second syntax above is preferred.

Lifting Edge Weight
~~~~~~~~~~~~~~~~~~~

In the concrete implementation of Dijkstra’s algorithm shown above, we
assumed the container associated with each vertex in the graph (i.e.,
the container obtained by ``G[u]``) provided tuples containing the
vertex id and the edge weight. In fact, there are numerous ways to
associate a weight with each edge. We could, for example, store an edge
index with each neighbor and use that to index into an array that we
also pass into ``dijkstra``. In such a case the (unconstrained)
prototype for the algorithm might be

.. code:: cpp

   template <class Graph, class Range>
   auto dijkstra(const Graph& G, vertex_id_t<Graph> s, Range wt);

The inner loop might then look like

.. code:: cpp

   for (auto&& e : G[u]) {
     auto v = target(e);
     auto w = wt[v];
     if (d[u] + w < d[v]) {
       d[v] = d[u] + w;
       pi[v] = u;          }}

To lift this version and the version with the directly-stored property
on edges, we introduce a ``weights`` callable WeighFunction as a
parameter at the interface of ``dijkstra``, using concepts to ensure
that the function meets the required use.

.. code:: cpp

   template <adjacency_list Graph, class WeightFunction>
   auto dijkstra(const Graph& G, vertex_id_t<Graph> s, WeightFunction wt);

In this case, the inner loop would look like

.. code:: cpp

   for (auto&& e : G[u]) {
     auto v = target(e);
     auto w = wt(e);
     if (d[u] + w < d[v]) {
       d[v] = d[u] + w;
       pi[v] = u;          }}

Non-Type Constraints
~~~~~~~~~~~~~~~~~~~~

We have already seen in lifting the edge weight that not all constraints
for an algorithm are encapsulated in the type requirements for the input
graph. There are other requirements that an algorithm may have that
cannot be captured as a type requirement, or as any compile-time
checkable requirement. For example, some algorithms, such as triangle
counting, may require that the edges within each neighborhood be sorted.
Or, as we have seen, directedness of a graph is not a type property, but
rather a runtime property. Such requirements become part of the
specification of the API, but cannot be made part of type checking. This
is similar to, say, ``binary_search`` in the C++ standard library, which
requires that the elements of the container to which it is applied be
sorted. Yet, there is no such thing as a sorted container type in the
standard library.

Other Concepts
--------------

So far we have developed a single concept (``adjacency_list``) and the
reader may ask how broad that concept is, given the wide variety of
potential graph algorithms. In fact, ``adjacency_list`` concept is
surprisingly broad in its applicability; only a few supplemental
concepts are required to cover all of the algorithms implemented in
NWGraph and, based on our analysis of the comprehensive Boost Graph
Library:cite:`Siek_Lee_Lumsdaine_2002`, probably all of the
algorithms that are likely to be implemented in NWGraph in the future.
This is perhaps not so surprising since the adjacency list
:math:`Adj(G)` is also the primary theoretical construct upon which the
majority of graph algorithms are built.

There are two additional concepts that we introduce briefly here which
we found necessary for algorithms in NWGraph: ``degree_enumerable`` and
``edge_list``. The former extends ``adjacency_list`` with the
requirement that there be a valid expession ``degree``, necessary in
some algorithms. The latter is basically a container of objects for
which ``source`` and ``target`` are valid expressions. Algorithms such
as Bellman-Ford and Kruskal’s MST use an edge list rather and adjacency
list:cite:`Cormen_2009`.

Our confidence that these concepts are sufficient is based on an
extensive study of the concepts in the Boost Graph
Library:cite:`Siek_Lee_Lumsdaine_2002`. The Boost Graph
Library (BGL) has five essential concepts that cover all of its
algorithms: ``VertexListGraph``, ``EdgeListGraph``, ``AdjacencyGraph``,
``IncidenceGraph``, and ``BiIncidenceGraph``. Of these, the design
decisions of NWGraph to require vertex identifiers to be indices
obviates ``VertexListGraph``. We don’t need to iterate through a list of
vertices provided by the graph. The NWgraph ``adjacency_list`` and
``degree_enumerable`` concepts subsume the essential functionality of
``AdjacencyGraph`` and ``IncidenceGraph``. ``adjacency_list`` does not
have a ``source`` function requirement, but that is in fact only rarely
used in the BGL algorithms requiring ``IncidenceGraph`` (and when it is
used, there are other ways of obtaining the same information). The
``BiIncidenceGraph`` concept is essentially a composite of a graph and
its transpose. This is obviated in the NWGraph design. Algorithms
requiring a graph and its transpose take them as two arguments rather
than one (there is no data that can be shared between a graph and its
transpose, so there is no loss of efficiency in that design decision).
Finally, the NWgraph ``edge_list`` concept is essentially identical to
the BGL ``EdgeListGraph`` concept.

