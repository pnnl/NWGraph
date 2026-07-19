.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

Practical Graph Theory
======================


Graphs are powerful abstractions for reasoning about entities in relationship.  Yet
graph algorithms require more than just graph models of problems domains -- they
require data structures over which they can operate (even more so for implementations
of graph algorithms in software).

By this we mean the following.  You will often see definitions for graphs 
(which we in fact use below)
along the lines of: 
A graph is a set :math:`G = \{ V, E \}` , where :math:`V`
is a finite set of vertices and :math:`E` is a finite set of edges, etc.  This is
fine, but one can't build algorithms or software with finite sets of things.  One
needs a more useful organization of these entities, such as an adjacency list.
Unfortunately, these more useful structures are also referred to as ``graphs''.
Moreover, the terminology used in theoretical presentations of algorithms, e.g., an
adjacency list, may not be at all what is actually used in a software implementation.


In this section, we briefly go through the terminology used in graph theory, but from
the point of view of knowing that we will ultimately be building a software
library for graph computing.
We start with the basic definition of a graph as a pair of finite
sets and then build up the particular organization structures that are needed to
describe graph algorithms and which we will reify in software to implement those
algorithms.  We then provide a high-level overview of the realization of those
structures in NW Graph.




Some Mathematical Terminology
-----------------------------

Abstractly, we define a graph :math:`G` as comprising two finite sets, 
:math:`G = \{ V, E \} ` ,
where the set :math:`V` is a set of entities of interest (''vertices'' or ``nodes'') and :math:`E`
is a set of pairs of entities from :math:`V` (''edges'' or ''links'').  Without loss of
generality we label the entities in :math:`V` as :math:`v_i` so that :math:`V = \{ v_0, v_1, \ldots
v_{n-1} \}`.  The set of edges (also labeled) can be constructed using the labeled
entities from :math:`V` so that :math:`E = \{ e_0, e_1, \ldots e_{m-1} \}`.  The edges may be
ordered pairs, denoted as :math:`(v_i, v_j)`, which have equality defined such that
:math:`(v_i,v_j) = (v_m,v_n) \leftrightarrow v_i = v_m \wedge v_j = v_n`. Or, the edges may
be unordered sets, denoted as :math:`\{v_i, v_j\}` which have equality defined as :math:`(v_i,v_j)
= (v_m,v_n) \leftrightarrow\left( v_i = v_m \wedge v_j = v_n\right) \vee \left( v_i =
v_n \wedge v_j = v_m\right)` .  If a graph is defined with ordered edges we say the
graph is directed; if the graph is defined with unordered edgese say the graph is
undirected.


Below are 
two examples of modeling a problem using a graph.  

In the first case, two-terminal circuit elements connect to each other at given nodes.  We thus model circuit connection points (also the terminology used in circuit modeling) as graph nodes, and the connections between them as edges.  In the case of circuits, orientation of circuit elements matters and so we may choose (at least at this stage of the modeling process) to use directed edges in the graph.  


.. list-table:: 
   :widths: 350 233 133

   * -
     -
     -
   * -
      Electrical circuit consisting of nodes and circuit elements.

     -
      Graphical representation of the electrical circuit.

     -
      Set-theoretical (mathematical) representation.

   * -
      .. figure:: ../_static/images/circuit.pdf
        :width: 400px
        :align: center
        :alt: alternate text
        :figclass: align-center

     -
      .. figure:: ../_static/images/circuit-graph.pdf
        :width: 200px
        :align: center
        :alt: alternate text
        :figclass: align-center

     -
      .. math::

         \small\begin{array}[t]{rcl}
         G & = & \{ V, E \} \\
         V & = & \{ \textrm{0}, \textrm{Vdd}, \textrm{n0}, \textrm{n1}, \textrm{n2}, \textrm{Vout} \} \\
         E & = & \{
         ( \textrm{n0}, \textrm{n1} ),
         ( \textrm{Vdd}, \textrm{0} ), \\
         &&\:\:
         ( \textrm{n0}, \textrm{Vdd} ),
         ( \textrm{n2}, \textrm{Vdd} ), \\
         &&\:\:
         ( \textrm{0}, \textrm{n2} ),
         ( \textrm{n2}, \textrm{Vout} ), \\
         &&\:\:
         ( \textrm{0}, \textrm{n0} ),
         ( \textrm{n2}, \textrm{n1} ) \}
	 \:&\:\\
	 \:&\:\\
         \end{array}



In the second case, we begin with a table of airports and the distance in kilometers between pairs of them.  We model this situation as a graph by identifying graph nodes with airports and graph edges with pairs of cities that are given as pairs in the distance table.  

.. list-table:: 
   :widths: 333 233 133

   * -
     -
     -

   * -
        Table of airport codes and distance in km between the airports.

     -
        Graphical representation of the tabular data.

     -
        Set-theoretical (mathematical) representation.

   * -
      .. figure:: ../_static/images/airport-tables.pdf
        :width: 200px
        :align: center
        :alt: alternate text
        :figclass: align-center

     -
      .. figure:: ../_static/images/airport-graph.pdf
        :width: 200px
        :align: center
        :alt: alternate text
        :figclass: align-center

     -
      .. math::

         \begin{array}{rcl}
         G & = & \{ V, E \} \\
         V & = & \{ \textrm{SEA}, \textrm{MSP}, \textrm{SLC}, \textrm{DTW}, \textrm{ATL}, \textrm{BOS} \} \\
         E & = & \{ 
         \{ \textrm{MSP}, \textrm{DTW} \}, 
         \{ \textrm{SLC}, \textrm{SEA} \}, \\
         &&\:\: 
         \{ \textrm{MSP}, \textrm{SLC} \}, 
         \{ \textrm{BOS}, \textrm{SLC} \}, \\
         &&\:\: 
         \{ \textrm{SEA}, \textrm{BOS} \}, 
         \{ \textrm{BOS}, \textrm{ATL} \}, \\
         &&\:\: 
         \{ \textrm{SEA}, \textrm{MSP} \}, 
         \{ \textrm{BOS}, \textrm{DTW} \} \} \\
	 \:&\:\\
	 \:&\:\\
         \end{array}



Unipartite Graphs and Bipartite Graphs
--------------------------------------

We may also have the case of a graph representing relationships between
two different kinds of entities. In that case we write
:math:`G\  = \ (U,\ V,\ E)`, where :math:`U` and :math:`V` are distinct
sets of different kinds of entities (and in general :math:`U` and
:math:`V` will not have the same cardinality). The set :math:`E` of
edges consists of ordered pairs :math:`(u,v)`, with :math:`u` coming
from :math:`U` and :math:`v` coming from :math:`V`. This kind of graph
is known as a *bipartite* graph; 
a graph with just a single vertex set may also be referred to as
a *unipartite* graph.

An example of a bipartite graph is the relationship between actors and the films in which they appear.

.. list-table:: 
   :widths: 225 125 450

   * -
     -
     -

   * -
        Table of movie titles and actors.

     -
        Graphical representation of the tabular data.

     -
        Set-theoretical (mathematical) representation.

   * -
      .. figure:: ../_static/images/title_principal.pdf
        :width: 200px
        :align: center
        :alt: alternate text
        :figclass: align-center

     -
      .. figure:: ../_static/images/title_principal_graph.pdf
        :width: 200px
        :align: center
        :alt: alternate text
        :figclass: align-center

     -
      .. math::

      .. figure:: ../_static/images/title_principal_set.pdf
        :width: 400px
        :align: center
        :alt: alternate text
        :figclass: align-center




Graph Traversal
---------------


To define algorithms on graphs and to be able to reason about those algorithms, we
need to define some useful representations for graphs—as mentioned before, one can’t
really do very much with abstract sets of vertices and edges.  And to do that we first
define some terminology regarding representations. Various characteristics of these
representations are what we use to express algorithms (still abstractly) but when
those algorithms are implemented as generic library functions, those characteristics
will in turn become the basis for the NW Graph function interface requirements (i.e.,
concepts).


One of the fundamental operations in graph algorithms is *traversal*.  That is, given
a vertex :math:`u`\ we would like to find the *neighbors* of :math:`u`, i.e., all
vertices :math:`v`\ such that the edge :math:`(u,\ v)` is in the graph. Then, for each
of those edges, we would like to find their neighbors, and so on. Different graph
algorithms arise from the order in which we decide to visit those neighbors (e.g.,
depth-first search or breadth-first search).  The representation that we define to
make traversal efficient is an *adjacency list.* We note that calling this structure a
"list" is standard terminology terminology for the abstract representation.  We aren’t
going to require that an actual linked-list be used--or that any other actual type be
used. Specific type requirements will be defined by the library concepts below.

There is an important transition in going from a graph (as a collection of vertex
objects and pairs of vertex objects) to an adjacency list.  Implied in using an
adjacency list for traversal is that we would like to be able “find the neighbors”
efficiently, i.e., in constant time, meaning we need to be able to take a vertex and
do a constant time lookup to get all of the neighboring vertices. Then, with what we
get back as the neighbors, we also need to use to look up more neighbors. In short,
regardless of what we consider to be the vertices or edges in our graph :math:`G`, an
adjacency list is something that stores indices which can be used to index into
itself.


Index Graphs
~~~~~~~~~~~~

Given a graph :math:`G\ = \ (V,\ E)`, we can define an adjacency-list representation
in the following way. Assign to each element of :math:`V`\ a unique index from the
range :math:`[ 0,\ |V|)` and denote the vertex identified with index :math:`i` as
:math:`V[ i].` We can now define a new graph with the same structure as :math:`G,` but
in terms of the indices in :math:`[ 0,\ |V|)`. Let the *index graph* of :math:`G` be
the graph :math:`= (,\ )`, where :math:`= [ 0,\ |V|)`\ and :math:`` consists of
:math:`\left| E \right|`\ pairs of indices from :math:`` such that a pair
:math:`(i,j)` is in :math:`` if and only if :math:`(V[ i],\ V[ j])` is in :math:`E.`
Which is all to say, the index graph of :math:`G`\ is the graph we get by replacing
all elements of :math:`G` with their corresponding indices.

This process is illustrated using the previous examples of a circuit and of airline routes.

.. list-table:: 
   :widths: 250 250

   * -
     -
     -

   * -
        Directed graph

     -
        Corresponding index graph

   * -
      .. figure:: ../_static/images/spice_pre_index.pdf
        :width: 225px
        :align: center
        :alt: alternate text
        :figclass: align-center

     -
      .. figure:: ../_static/images/spice_index.pdf
        :width: 225px
        :align: center
        :alt: alternate text
        :figclass: align-center


.. list-table:: 
   :widths: 250 250

   * -
     -
     -

   * -
        Undirected graph

     -
        Corresponding index graph

   * -
      .. figure:: ../_static/images/airport_pre_index.pdf
        :width: 225px
        :align: center
        :alt: alternate text
        :figclass: align-center

     -
      .. figure:: ../_static/images/airport_index.pdf
        :width: 225px
        :align: center
        :alt: alternate text
        :figclass: align-center



Of course, we don’t need an underlying graph to define what an index graph itself
is. We can say that an index graph :math:`G`\ = (V, E) is any graph with the property
that the vertex set is a set of contiguous indices, with :math:`V = [ 0,|V| -
1)`. Since an index graph is just a graph, in cases where the context is clear, we may
refer to an index graph simply as a graph. We note that an adjacency list can only be
defined over an index graph.


Adjacency Lists
~~~~~~~~~~~~~~~

An adjacency list of an index graph :math:`G = (V,E)` is an array
:math:`\text{Adj}` of size :math:`|V|`\ (the array is indexed from 0 to
:math:`|V| - 1`). Each entry :math:`Adj[ u]` in the array is
a container of all the vertices :math:`v`\ for which :math:`(u,v)`\ is
contained in :math:`\text{E.}` This structure (an adjacency list of an
index graph, or an index adjacency list) is the fundamental structure
used by almost all graph algorithms.


.. list-table:: 
   :widths: 600

   * -
     -
     -

   * -
        Airport adjacency list

   * -
      .. figure:: ../_static/images/airport_adjacency_list.pdf
        :width: 600
        :align: center
        :alt: alternate text
        :figclass: align-center



.. list-table:: 
   :widths: 600

   * -
     -
     -

   * -
        Circuit adjacency list

   * -
      .. figure:: ../_static/images/circuit_adjacency_list.pdf
        :width: 600
        :align: center
        :alt: alternate text
        :figclass: align-center


**NB (1):** Although the standard term for this kind of abstraction is “adjacency
list”, and although it is often drawn (as we do here) with linked lists as elements,
it is not necessary that this abstraction be implemented this way. What is important
is that the items that are stored (vertex indices) can be used to index into the
adjacency list to obtain other lists of neighbors.

**NB (2):** The index adjacency list does not store edges per se and
therefore the index adjacency list is neither inherently directed nor
undirected. Again, for a given vertex :math:`u,`\ the container
:math:`Adj[ u]` contains the vertex :math:`v`\ if the edge
:math:`(u,\ v)` is contained in :math:`\text{E.}` This means that for a
directed graph with edge :math:`(u,v)` in :math:`,`
:math:`EAdj[ u]` will contain :math:`\text{v.}` For an
undirected graph with edge :math:`(u,\ v)` is contained in :math:`E,`
:math:`Adj[ u]` will contain :math:`v`\ **and**
:math:`Adj[ v]` will contain :math:`\text{u.}` Directedness
of the original graph is thus made manifest in the *values* stored in
the index adjacency list. But there is nothing about the structure or
semantic properties of the adjacency list itself that reflects the
directedness (or undirectedness) of the original graph.

Summarizing some points to note about computing with graphs, based on
the preceding.

-  The vertex set in an index graph is a contiguous range of identifiers
      :math:`[ 0,\ |V|)`

-  There are no vertices per se in an index graph (we don’t materialize
      :math:`V`).

-  Edges in an index graph are pairs of vertex identifiers.

-  An adjacency list does not store vertices. An adjacency list is a
      structured organization of the relationship information contained
      in the edges.

-  An adjacency list does not store edges.

-  Vertex ids obtained from iterating neighbor lists can be used to
      index back into the adjacency list.

-  An adjacency is neither ordered nor unordered (but is over a graph
      that may be ordered or unordered).

And, finally, a meta-point. Although graphs and graph algorithms are the
central focus of a graph library, graphs in some sense are ephemeral.
The thing that will be computed on by a graph algorithm is an organized
collection of vertex identifiers that were generated to refer back to
some real data. Graph texts will often use a phrase like “edges and
vertices have properties” but a more correct statement would be
“properties have edges and vertices.”
