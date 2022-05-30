.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:related`:

Related Libraries and Toolkits
==============================

This section explores the landscape of related graph libraries and
frameworks. Each of the libraries or tools discussed in this section
make different design tradeoffs regarding usability, extensibility, and
performance. Though few of the tools in this section (with the exception
of BGL) aimed to fill the role of an STL graph library, they all
contribute to a greater understanding of graph library design.

**Generic C++ Graph Libraries.** The Boost Graph Library
(BGL) :cite:`Siek_Lee_Lumsdaine_2002` and LEMON graph
library :cite:`dezsHo2011lemon` both contributed to the
development of generic graph algorithms in C++. BGL proposed algorithm
templates that could be used on a variety of graph types (which could be
generated using BGL’s graph type generator), e.g., vector of vectors,
vector of lists, etc. Vertices and edges were allowed to be arbitrary
types accessed via property maps which could be stored internally or
externally to the graph. The default graph algorithms could be
customized using visitor objects, which allowed users to use existing
data access patterns to do additional work, for instance recording the
timestamp a vertex is touched in a BFS search. The LEMON graph library
shared many of these features. Both libraries advertise algorithms that
work with user defined graphs, so long as they conform to a certain
interface.

Some of these features had shortcomings that limited their use. The
visitor objects are difficult to use, both from a programming and
algorithmic design perspective. Property maps are a powerful programming
abstraction, but in addition to being difficult to use, could lead to
performance issues. The type of LEMON’s graph adaptors are different
from the original graph type being adapted, and their use as graphs is
only supported in limited ways. As mentioned in the previous section, a
major shortcoming of these designs is the difficulty of using custom
data structures. In order to adapt an existing user-defined data
structure, the BGL interface requires overloading several global free
functions. These mostly include accessors, mutators, and iterators for
edges and vertices. An assumption is placed on the graph container type
being adapted that it will have much of the same behavior as the built
in BGL container types. Furthermore both libraries lack newer features
in C++ such as constexpr, variadic templates, automatic type deduction,
execution policies, etc.

**HPC Graph Frameworks** There are several graph frameworks designed to
maximize performance in distributed memory or shared memory, such graph
frameworks include Parallel Boost Graph Library
(PBGL) :cite:`Lifting_douglas_2005`,
Galois :cite:`kulkarni2007`,
Ligra :cite:`shunppopp13`,
Giraph :cite:`shaposhnik_practical_2015`,
Gunrock :cite:`wang2016gunrock`,
GraphIt :cite:`og2020`, etc. The contributions of these
frameworks are typically a computational model for parallel processing
of graphs, with less emphasis on the usability or extensibility of graph
algorithms or containers. A thorough evaluation of several well-known
parallel graph frameworks can be found
in :cite:`azad2020evaluation`.
