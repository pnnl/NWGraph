.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

NWGraph: The Northwest Graph Library
====================================

Graphs are powerful mathematical tools for reasoning about the
relationships between given entities, focusing on the *structure and
characteristics of the relationships*, independent of what the entities
and the relationships actually are. Consequently, results from graph
theory can be applied to any actual sets of data for which relationships
between elements can be established. Internet packet routing, molecular
biology, electronic design automation, social network analysis, and
search engines are just some of the problem areas where graph theory is
regularly applied. The general applicability we find in graph theory –
the *genericity*, if you will – is a goal for software libraries as well
as mathematical theories; graph algorithms and data structures
(collectively, “graphs”) would seem to be ideally suited for software
reuse.

Realizing a truly generic library for graphs has significant challenges
in practice. Graphs in theory are useful because they are abstract, but,
in practice, they have to be made concrete when used to solve an actual
problem. That is, graphs in practice do not manifest themselves in the
abstract form to which theory and abstract algorithms are applied.
Rather, they are often encoded in some domain-specific form or latent in
problem-related data. And even if a domain programmer constructs a graph
from their data, the domain-specific graph data structure might not be
compatible with the API of a given graph library.

The celebrated Standard Template Library :cite:`STL` (now
part of the C++ standard library) addressed this problem for fundamental
algorithms and abstract containers of data elements. With the STL,
*generic programming* emerged as a software-development sub-discipline
that focused on creating frameworks of reusable and composable
libraries. Fundamental to the philosophy of generic programming is that
algorithms should be able to be composed with arbitrary *types*, notably
types that may have been developed completely independently of the
library. To achieve this goal, generic algorithms are specified and
written in terms of *abstract properties of types*; a generic algorithm
can be composed with any type meeting the properties that it depends on.
Philosophically, generic programming goes hand-in-glove with the
abstraction process inherent in graph theory. Graphs are abstract models
of entities in relationship – a graph algorithm should be able to
operate directly on the entities and relationship in a programmer’s
data.

It is not just the philosophy of generic programming from the STL that
can be leveraged to develop a generic graph library. In fact, an
important principle upon which our work is based is that **the standard
library already contains sufficient capability to support graph
algorithms and data structures.** The *type requirements for generic
graph algorithms* can be expressed using existing type requirement
machinery for standard library algorithms and useful and efficient graph
algorithms can be implemented based on these requirements.

We apply this principle to develop NWGraph, a generic library of
algorithms for graph computation that are independent of any particular
data structure (in particular, independent of any particular ``Graph``
data structure). Following current generic library practice, NWGraph
algorithms are organized around a minimal set of common requirements for
their input types (these requirements are formalized in the form of
C++20 *concepts*).

The foundation of the paper is a requirements analysis from which we
derive a uniform set of type requirements for graph algorithms, which we
subsequently reify as C++ concepts. Based on this foundation, we
construct the primary components of NWGraph: **algorithms**, defined and
and implemented using our concepts; **adaptors**, for converting one
representation of a graph into another and for enabling structured
traversals, and **data structures** that model our foundational
requirements.

NWGraph contains the following innovations:

-  A concept taxonomy (expressed using C++20 concepts) for specifying
   graph algorithm requirements;

-  Characterization of graphs using standard library concepts (as a
   random access range of forward ranges);

-  A rich set of range adaptors for accessing and traversing graphs;

-  An API designed to fully support modern idiomatic C++;

-  An efficient and fully parallelized (using C++ execution policies and
   Intel’s Threading Building Block) implementation; and

-  Maximum compatibility with third-party data structures and
   algorithms.

