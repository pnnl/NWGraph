.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:exp_evaluation`:

Performance Evaluation
======================

In this section, we discuss our experimental results. First we evaluate
the abstraction penalty for different ways of iterating over a graph as
a range of ranges. Next we evaluate the abstraction penalty for
representing a graph with different data structures. Finally, we compare
the performance of our NWGraph library with 3 other well-known graph
frameworks: GAP graph benchmark suite :cite:`GAP`, Galois
:cite:`kulkarni2007` graph library and GraphIt
 :cite:`graphit:2018` domain-specific language for graphs.

Experimental Setup
------------------

Abstraction penalty benchmarks were run on 2019 MacBook Pro with 2.4 GHz
8-Core Intel® Core i9 processor with 64 GB DDR4 memory running at
2.6GHz. All performance measurements were collected on Intel® Xeon®
-based servers. Each server contains two Intel® Xeon® Platinum 8153
processors, each with 16 physical cores (32 logical cores) running at
2.0 GHz. Each processor has 22 MB L3 cache. The total system memory of
each server is 384 GB DDR4 running at 2.6 GHz. We used GCC 10 with
“-Ofast” and TBB 2020.3 to compile.

.. container:: figure*

   .. figure:: imgs/abp-spmv-intel.pdf
      :alt: Different data access abstractions (``iterator``\ s,
        ``range``\ s, ``std::for_each`` and ``neighbor_range`` adaptor)
        with their abstraction penalties measured relative to a raw for
        loop implementation. There is no significant performance penalty
        relative to the raw loop implementation.
      :name: fig:abp_spmv

      Different data access abstractions (``iterator``\ s, ``range``\ s,
      ``std::for_each`` and ``neighbor_range`` adaptor) with their
      abstraction penalties measured relative to a raw for loop
      implementation. There is no significant performance penalty
      relative to the raw loop implementation.

   .. figure:: imgs/abp-containers-intel.pdf
      :alt: Measured abstraction penalty for the SpMV benchmark with
        graphs represented by different containers using ``iterator``
        based ``for`` loop. The execution time has been normalized w.r.t
        to the execution time of SpMV with graphs represented as
        ``struct_of_array`` (lower is better).
      :name: fig:abp_containers

      Measured abstraction penalty for the SpMV benchmark with graphs
      represented by different containers using ``iterator`` based
      ``for`` loop. The execution time has been normalized w.r.t to the
      execution time of SpMV with graphs represented as
      ``struct_of_array`` (lower is better).

.. _`sec:abp_results`:

Abstraction Penalty
-------------------

Using a range-based interface introduces a variety of different ways to
iterate through a graph (including the use of graph adaptors). While
ranges and range based for loops are useful programming abstractions, it
is important to consider any performance abstraction penalties
associated with their use. We benchmark these penalties to ensure they
will not significantly limit performance compared to a “raw for loop”
implementation. For example let us consider the sparse matrix-dense
vector multiplication (SpMV) kernel used in page rank, which multiplies
the adjacency matrix representation of a graph by a dense vector
:math:`x` and stores the result in another vector :math:`y`. Using a
compressed sparse row (CSR) data structure to store the adjacency
matrix, a raw for loop implementation would access the indices and
weights of edges with pointers into the CSR data structure.

.. code:: c++

   // Raw for loop SpMV
   auto ptr = G.indices_.data();
   auto idx = std::get<0>(G.to_be_indexed_).data();
   auto dat = std::get<1>(G.to_be_indexed_).data();
   for (vertex_id_t i = 0; i < N; ++i) {
     for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
       y[i] += x[idx[j]] * dat[j]; }}

However NWGraph does not assume this underlying CSR structure, and would
prefer to write these algorithms more generically with iterator based or
range based for loops shown below. Note that the previous raw loop
implementation had access to information that the SpMV kernel does not
actually need, which is random access into a vertex’s incidence list.
The incidence list only needs to be traversed in some order to produce
the desired result.

.. code:: c++

   // Iterator based for loop SpMV
   vertex_id_t k = 0;
   for (auto i = G.begin(); i != G.end(); ++i) {
     for (auto j = (*i).begin(); j != (*i).end(); ++j) {
       y[k] += x[get<0>(*j)] * get<1>(*j); }
     ++k; }
   // Range based for loop SpMV
   vertex_id_t k = 0;
   for (auto&& i : G) {
     for (auto&& [j, v] : i) {
       y[k] += x[j] * v; }
     ++k; }

Iterators can also be used to process the edges with ``std::for_each``.

.. code:: c++

   // STL for_each SpMV
   std::for_each(G.begin(), G.end(), [&](auto&& e) {
     y[std::get<0>(e)] += x[std::get<1>(e)] * std::get<2>(e); });

Finally, with one of our range adaptors, ``neighbor_range``, we can
easily access the indices and the neighbors of it.

.. code:: c++

   // Neighbor range based for loop (SpMV)
   for (auto&& [i, neighbors] : neighbor_range(G)) {
     for (auto&& [j, v] : neighbors) {
       y[i] += x[j] * v; }}

There are even more combinations of these traversals that are omitted
for lack of space (combinations of ranges and iterators, with and
without compound initializers, ``auto`` vs ``auto&&`` etc.).

To experimentally evaluate the abstraction penalty, we consider SpMV
with three graphs with different underlying topologies from the
SuiteSparse matrix collection: circuit5M, GAP-road, and hugebubbles.
These were chosen because they have similar numbers of edges (30M to
60M) and the benchmarks run in comparable time. Timing results were
averaged over 5 runs of each benchmark.   shows the results of the
different data access abstractions relative to the raw loop timing, for
each benchmark. Bars significantly higher than the raw for loop bar
would indicate a significant performance penalty. None of the
abstraction methods incurs a significant performance penalty relative to
the raw loop implementation.

We also evaluated the abstraction penalty incurred for storing a graph
in different containers. In particular, we have selected
``struct_of_array``, ``vector_of_vector``, ``vector_of_list``,
``vector_of_forward_list`` containers. Note that all of these containers
meet the requirement of our ``graph`` concept. We consider SpMV
benchmark implemented with iterator based for loop with circuit5M,
GAP-road, and hugebubbles datasets.   shows the performance of SpMV with
different containers. The execution time is normalized relative to the
execution time of SpMV with ``struct_of_array`` container. As can be
observed from  , SpMV with ``struct_of_array`` performs best, followed
by ``vector_of_vector``. ``struct_of_array`` representation is
cache-friendly and supports random access of the outer and inner range
efficiently.

.. _`sec:performance`:

Performance on Large-Scale Graphs
---------------------------------

In addition to being generic, NWGraph is intended to be a
high-performance library and includes parallel implementations of a
number of its algorithms. Parallelization is effected using C++ standard
library execution policies and with Intel’s Threading Building Blocks
(TBB) (:cite:`tbbrepo`).

To demonstrate NWGraph’s performance on large-scale graphs, we evaluate
and compare the performance of NWGraph with three well-known graph
frameworks: GAP :cite:`GAP`,
Galois :cite:`10.1145/2517349.2522739` and
GraphIt :cite:`graphit:2018`. The evaluation is intended to
assess the performance of various parallel graph algorithms available in
NWGraph in the context of other HPC graph frameworks. Since BGL is
single-threaded, we did not include it for comparison. Overall, NWGraph
leverages TBB concurrent data structures for maintaining the internal
states of different graph algorithms. In addition, for workload
distribution among the threads, NWGraph can either use ``block`` range
from TBB, our customized ``cyclic`` range adaptor, or C++ parallel
execution policy (``std::execution::par``,
``std::execution::par_unseq``) whenever appropriate. All experiments are
conducted on 32 physical cores.

For our experiments, we chose five representative datasets according to
the GAP benchmark suite (:cite:`GAP`). These datasets ()
have diverse structural properties and have been collected from various
application domains. We select six different graph algorithms
(Betweenness Centrality, Breadth-first Search, Connected Components,
Page Rank, Single Source Shortest Path, and Triangle Counting) that are
common across these graph frameworks.

.. container:: figure*

   .. figure:: imgs/BC_perf.pdf

   .. figure:: imgs/BFS_perf.pdf

   .. figure:: imgs/CC_perf.pdf	       

   .. figure:: imgs/PR_perf.pdf	       

   .. figure:: imgs/SSSP_perf.pdf	       

   .. figure:: imgs/TC_perf.pdf	       


The performance of different graph frameworks is shown above . We
summarize our observations as follows:

-  With Web and kron datasets, which have skewed degree distribution,
   our triangle counting (TC) and Gauss-Seidel page rank algorithms in
   NWGraph outperform other frameworks. Except for the road network
   input, for both of these graph problems, NWGraph performs comparably
   with other inputs. The performance of NWGraph can be attributed to
   the cyclic range adaptor, which helps to achieve better load balance
   among the threads for graphs with skewed degree distribution. TC also
   includes pre-processing techniques such as relabeling the vertices by
   degree (the pre-processing time is included in the reported
   normalized execution time).

-  NWGraph also runs faster with Web, Twitter and Kron datasets
   (power-law graphs) for Betweenness Centrality (BC) algorithms.

-  NWGraph suffers performance with bounded graphs such as road network
   input for SSSP due to limited parallelism available and due to the
   uniform degree distribution in these types of graphs. Overall,
   NWGraph performs better or comparable to other graph frameworks.

-  For connected component (CC), all frameworks except GraphIt implement
   Afforest algorithm :cite:`sutton2018optimizing`. Hence GraphIt
   performs worse with all inputs for CC.
