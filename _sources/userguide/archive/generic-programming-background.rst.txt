.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:generic-programming-background`:

Generic Programming in C++20
============================

.. _`sec:gen_programming`:

Generic Programming
-------------------

Generic programming is a software development paradigm inspired by the
organizational principles of
mathematics :raw-latex:`\cite{stepanov2014mathematics}`. That is, a
generic library comprises a framework of algorithms in a problem domain,
based on a systematic organization of common type requirements for those
algorithms. The type requirements themselves, specified as *concepts*
are part of the library as well, and provide the interface that enables
composition of library components with other, independently-developed,
components. The ``iterator`` concept taxonomy, for example, was the
foundation upon which the STL was
organized :raw-latex:`\citep{STL,Musser89}`.

.. code:: cpp

   int sum(int *array, int n) {
     int s = 0;
     for (int i = 0; i < n; ++i) {
       s = s + array[i]; }
     return s; }

.. code:: cpp

   double sum(node *bgn, node *end) {
     double s = 0;
     while (bgn != end) {
       s = s + bgn->data;
       bgn = bgn -> next;}
     return s; }

Generic algorithms (that is, algorithms in a generic library) are
designed so that the requirements they impose on types are as minimal as
possible without compromising efficiency, thus enabling the widest scope
of potential composition, and therefore, reuse. Generic algorithms are
derived from concrete ones, which are gradually made more generic by
removing (“lifting”) unnecessary requirements. This process continues as
long as instantiation of the generic algorithm with concrete types
remains as efficient as the equivalent concrete algorithm would have
been. Generic libraries do not tolerate abstraction penalty.

We emphasize again that in a generic library, the requirements on
algorithms lead to the concepts, which in turn represent the interface
to the library. The goal is to create an efficient framework of
highly-reusable algorithms that can be composed with arbitrary
third-party components – *not* to start with a data type intended to
meet all needs (even in the guise of a concept) and then define
conforming algorithms.

The Generic Programming Process
-------------------------------

Lifting
~~~~~~~

The first (and major) phase of the generic programming process is
sometimes known as “lifting“ where we create generic algorithms through
a process of successive generaliation. That is, the process is

#. Study the concrete implementation of an algorithm

#. Lift away unnecessary requirements to produce a more abstract
   algorithm

#. Repeat the lifting process until we have obtained a generic algorithm
   that is as general as possible but that still instantiates to
   efficient concrete implementations

#. Catalog remaining requirements and organize them into concepts

.. code:: cpp

   template <class Iter, class T>
   T sum(Iter first, Iter last, 
         T s) {
     while (first != last) {
       s = s + *first++;}
     return s; }

.. code:: cpp

   template <class Iter, class T, class Op>
   T accumulate(Iter first, Iter last, 
                T s, Op op) {
     while (first != last) {
       s = op(s, *first++); }
     return s; }

Fig. `[fig:sum] <#fig:sum>`__ shows two concrete implementations of a
``sum`` algorithm. The first steps through an array of integers,
indexing into the array at each step and summing the resulting value
into ``s``. The second “cdrs” through a linked list of ``node``
structures, obtaining the value stored in the ``data`` field and summing
the obtained value into ``s``.

The authors of the STL realized the commonality of traversal and element
access across most basic computer science algorithms. The requirements
for traversal and access were generalized and unified into a hierarchy
of type requirements known as iterators :raw-latex:`\cite{STL}`.

An iterator-based algorithm for summing elements in a container is shown
in Fig. `[fig:accum] <#fig:accum>`__ (left). Note that this single
parameterized algorithm replaces the ``sum`` algorithms shown in
Fig. `[fig:sum] <#fig:sum>`__ (and more). The process of summation can
further be generalized by the introduction of function objects, as is
shown in lifting ``sum`` to ``accumulate`` in the right of
Fig. `[fig:accum] <#fig:accum>`__.

Specialization
~~~~~~~~~~~~~~

In generic programming, the dual to lifting is *specialization.* That
is, once an algorithm is lifted and made generic, it is specialized
through composition with a concrete data type to realize a concrete
implementation of the algorithm. Fig. `[fig:spec] <#fig:spec>`__ shows
two examples of usage of the generic ``sum`` and ``accumulate`` that we
created, composing them with the original data structures we lifted
from. We note that the linked list example uses the equivalent
linked-list implementation from the standard library, as that provides
the necessary iterator interfaces for the generic algorithms to use.

.. code:: cpp

   int* array = new int [10];
   int result =
       sum(array, array+10, 0);

.. code:: cpp

   std::forward_list<double> ptr;
   double result = accumulate(ptr, nullptr, 
                   0.0, std::times<double>);

Now, there is a crucial requirement that is part of specialization. In
generic programming, we don’t just require that when we have a lifted
algorithm that we can compose with the data types that we lifted from.
In addition to that basic requirement, we also require that *there is
zero abstraction penalty*. That is, the specialized generic algorithm
should provide exactly the same performance as the concrete algorithm
from which it was lifted, when composed with the original types that
were lifted. With modern compilers and libraries, this requirement is
actually met, and is one of the reasons that libraries such as the C++
standard library have been so successful in practice.

Concepts in C++20
-----------------

In generic programming, concepts consist of valid expressions and
associated types, which define a family of allowable types admissable
for composition with generic algorithms. Introduced as a language
feature for C++20, concepts are an extension of templates that constrain
the set of types that can be substituted for class and function template
arguments. Concepts formalize and make explicit—-at the language
level—-properties about types that were previously informal and/or
implicit. This development has been instrumental in the notable
development of the ranges algorithm library taxonomy, serving as the
link between generic algorithm interface and implementation.

A ``concept`` definition declares a set of requirements on types. There
are four types of requirements:

-  A simple requirement is an arbitrary expression statement. The
   requirement is that the expression is valid.

-  A type requirement is the keyword ``typename`` followed by a type
   name, optionally qualified. The requirement is that the named type
   exists.

-  A compound requirement specifies a conjunction of arbitrary
   constraints such as expression constraint, exception constraint, and
   type constraint, etc.

-  A nested requirement is another requires-clause, terminated with a
   semicolon. This is used to introduce predicate constraints expressed
   in terms of other named concepts applied to the local parameters.

.. code:: cpp
   :number-lines:

   template <class I>
   concept proto_input_iterator =
     requires(I i) {
       typename std::iter_value_t<I>;
       typename std::iter_reference_t<I>;
       { *i } -> std::same_as<std::iter_reference_t<I>>; !\label{code:iterator:dereference}!
       { ++i } -> std::same_as<I&>; !\label{code:iterator:postincrement}!
       i++;!\label{code:iterator:preincrement}!  };

Fig. `[fig:iterator-concepts] <#fig:iterator-concepts>`__ shows the
skeleton of the C++ concept definition for ``input_iterator``. As hinted
in our example, this concept specifies that an ``input_iterator`` can be
de-referenced with ``operator*``
(line `[code:iterator:dereference] <#code:iterator:dereference>`__) and
incremented with ``operator++``
(lines `[code:iterator:postincrement] <#code:iterator:postincrement>`__
and `[code:iterator:preincrement] <#code:iterator:preincrement>`__).
Additionally, the concept specifies two associated types:
``std::iter_value_t<I>`` and ``std::iter_reference_t<I>``.
Line `[code:iterator:dereference] <#code:iterator:dereference>`__ also
indicates that the expression ``*i`` returns the same type as
``std::iter_reference_t<I>``. Again, this example is abbreviated for
purposes of illustration. A complete description of the C++20 standard
library concepts (including the iterator hierarchy) can be found online
at ``https://en.cppreference.com/w/cpp/concepts``.

**Remark.** C++20 concepts are similar to type constraints in D
programming language. They are a way to constrain the types allowed
based on their properties. Concepts, as type constraints, support static
checking of instantiations during compilation time. If the concepts are
not specified for a template type, any type passed as an argument would
be valid at compilation time, and can potentially result in a run-time
error if the type does not meet certain properties. Ideas that are
closely related to concepts exist in other languages too, such as
Haskell typeclasses, Rust traits, Swift protocols, etc. However, a
subtle difference between these features and concepts is that these
“type classes” are a form of “consent”, rather than
“constraints” :raw-latex:`\citep{conorconcepts}`. A more detailed
discussion about the differences between (pre-C++20) concepts and
Haskell type classes can be found
in :raw-latex:`\citep{bernardy2010generic}`. An extensive comparative
study on support for generic programming in a variety of different
languages can be found in
 :raw-latex:`\citep{garcia03:_comparative_study}`, which was
subsequently extended to eight languages
in :raw-latex:`\citep{garcia07:_extended_comparative_study}`.

Ranges in C++20 
---------------

Essentially, ranges can be considered to be an abstraction of the notion
of a container, i.e., a collection of items that can be iterated over.
The new C++20 Ranges library :raw-latex:`\citep{niebler2018one}` adds
support for operating on ranges of elements. Ranges provide a way to
make STL algorithms *composable* and improve the readability and
writability of C++ code. Ranges consist of a pair of begin and end
iterators that are not required to be the same type. An example of using
``ranges`` is:

.. code:: cpp

   std::vector<int> v { /* ... */ }
   std::min_element(v.begin(), v.end());    // iterator interface
   std::ranges::min_element(v);             // ranges interface

In the first case, the generic ``min_element`` function is called with
an iterator pair (``begin`` and ``end`` of the container ``v``). In the
second case, ``min_element`` function is called directly with ``v`` as
the parameter, as a ``std::vector`` is a range (specifically, it
satisfies the requirements for the ``random_access_range`` concept.

C++20 ranges are defined in terms of C++20 concepts. A ``std::range``
itself is a very straightforward concept:

.. code:: cpp

   template< class T >
   concept range = requires(T& t) {
     ranges::begin(t);
     ranges::end  (t); };

It has two valid expressions: ``begin`` and ``end``. The
``std::input_range``, which abstracts containers that have forward
iterators, is thus defined:

.. code:: cpp

   template<class T>
   concept input_range = ranges::range<T>
       && std::input_iterator<ranges::iterator_t<T>>;

This definition states that an ``input_range`` is a ``range`` and that
furthermore, the iterator type associated with that range meets the
requirements of the ``std::input_iterator`` concept.

Related to graphs, two range concepts of particular relevance include
``ranges::forward_range``, which allows iteration over a collection from
beginning to end multiple times (as opposed to an input iterator which
is only guaranteed to be able to iterator over a collection once) and
``ranges::random_access_range``, which further allows indexing into a
collection with ``operator[]`` in constant time.

*Range adaptors*, alternatively known as *views*, can be considered as
wrappers around another range, without mutating or copying the original
range.
