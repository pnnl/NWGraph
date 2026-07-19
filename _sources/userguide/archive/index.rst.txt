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

By this we mean the following.  You will often see definitions like the following
(which we in fact use below): A graph is a set :math:`G = \{ V, E \},`
where :math:`V`
is a finite set of vertices and :math:`E` is a finite set of edges, etc.  This is
fine, but one can't build algorithms or software with finite sets of things.  One
needs a more useful organization of these entities, such as an adjacency list.
Unfortunately, these more useful structures are also referred to as ``graphs''.
Moreover, the terminology used in theoretical presentations of algorithms, e.g., an
adjacncy list, may not be at all what is actually used in a software implementation.


In this section, we briefly go through the terminology used in graph theory, but from
the point of view of ones knowing that we will ultimately be building a software
library for graph computing.
We start with the basic definition of a graph as a pair of finite
sets and then build up the particular organization structures that are needed to
describe graph algorithms and which we will reify in software to implement those
algorithms.  We then provide a high-level overview of the realization of those
structures in NW Graph.




Some Mathematical Terminology
-----------------------------

Abstractly, we define a graph :math:`G` as comprising two finite sets, 
:math:`G = \{ V, E \},` 
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



From Graph Theory to Programming with Graphs
============================================



Programming Abstractions
========================



Algorithms
----------



Concepts
--------



Data Structures
---------------



Some Functions
--------------

