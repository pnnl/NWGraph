.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:algorithms`:

NWGraph Algorithms
==================

Algorithms in NWGraph constitute the core of our library. NWGraph
includes a broad classes of algorithms (sequential and parallel) for
different graph problems, including graph traversal (BFS, SSSP),
analytics (PageRank, Jaccard similarity, connected components), motif
counting (triangle counting, betweenness centrality), network flow
(maximum flow) etc. The problem definitions and graph algorithms for
each of these graph problems implemented in NWGraph are tabulated below.

.. table::
   :width: 100%
   :widths: 35 65
   :class: tight-table

   +------------------------------+--------------------------------------+
   | Algorithm                    | Definition                           |
   +==============================+======================================+
   | Breadth-first search         | Traverses a graph in breadth-fist    |
   |                              | search order from a given source.    |
   |                              | Implementation includes: top-down,   |
   |                              | bottom-up and                        |
   |                              | direction-optimized                  |
   |                              | :cite:`Beamer-DOBFS`                 |
   |                              | algorithms.                          |
   +------------------------------+--------------------------------------+
   | Depth-first search           | Traverses a graph in depth-first     |
   |                              | search order from a given source.    |
   +------------------------------+--------------------------------------+
   | Single-source shortest paths | Finds the shortest distance paths    |
   |                              | from a given source to all other     |
   |                              | vertices in a graph.                 |
   |                              | :math:`\Delta`-stepping              |
   |                              | algorithm:cite:`MEYER2003114` is     |
   |                              | implemented.                         |
   +------------------------------+--------------------------------------+
   | Connected component          | Finds connected components in a      |
   |                              | graph. Implementations include       |
   |                              | Afforest:cite:`sutton2018optimizing`,|
   |                              | Shiloach-Vishkin                     |
   |                              | :cite:`shiloach1980log`,             |
   |                              | BFS-based :cite:`shun_simple_2014`   |
   |                              | and                                  |
   |                              | minimal label                        |
   |                              | propagation                          |
   |                              | :cite:`orzan_distributed_2004`       |
   |                              | :cite:`yan_pregel_2014`              |
   |                              | algorithms.                          |
   +------------------------------+--------------------------------------+
   | PageRank                     | Compute the importance of each       |
   |                              | vertex in a graph. Implements the    |
   |                              | Gauss-Seidel                         |
   |                              | algorithm                            |
   |                              | :cite:`arasu2002pagerank`            |
   +------------------------------+--------------------------------------+
   | Triangle counting            | Counts the number of triangles in a  |
   |                              | graph. Implements algorithms         |
   |                              | discussed in                         |
   |                              | :cite:`triangle_lumsdaine_2020`      |
   +------------------------------+--------------------------------------+
   | Betweenness centrality       | Measures how many times each vertex  |
   |                              | lies on the shortest paths to other  |
   |                              | vertices. Brandes                    |
   |                              | Algorithm :cite:`brandes_bc` has     |
   |                              | been implemented.                    |
   +------------------------------+--------------------------------------+
   | Maximum flow                 | Given a source and a sink, find      |
   |                              | paths with available capacity and    |
   |                              | push flow through them until there   |
   |                              | are no more paths available.         |
   |                              | Implements Edmonds-Karp algorithm.   |
   +------------------------------+--------------------------------------+
   | K-core                       | Finds the subgraph induced by        |
   |                              | removing all vertices with degree    |
   |                              | less than k.                         |
   +------------------------------+--------------------------------------+
   | Jaccard similarity           | Computes the Jaccard similarity      |
   |                              | coefficient of each pair of vertices |
   |                              | in a graph.                          |
   +------------------------------+--------------------------------------+
   | Graph coloring               | Assign a color to each vertex in the |
   |                              | graph so that no two neighboring     |
   |                              | vertices have the same color.        |
   |                              | Implements Jones-Plassmann           |
   |                              | algorithm                            |
   |                              | :cite:`jones1993parallel`            |
   +------------------------------+--------------------------------------+
   | Maximal independent set      | Graph coloring with two colors.      |
   +------------------------------+--------------------------------------+




Parallelization
---------------

NWGraph leverages existing parallelization support in the C++ standard
library for implementing different parallel graph algorithms. However,
to circumvent some of the limitations of the C++ standard library for
parallelization, we alternatively consider Intel’s Threading Building
Blocks (TBB) for performance.


Parallelization with std Execution Policies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In NWGraph, parallel algorithms for different graph kernels described
in  are implemented with ``std::execution::par`` (parallel policy) and
``std::execution::par_unseq`` (parallel unsequenced policy) provided to
the ``std::for_each`` construct. Updating shared variables in an
algorithm relies on the ``std::atomic`` operations library. For example,
a parallel triangle counting algorithm using parallel ``std::execution``
policy is shown in .

.. code:: cpp
   :number-lines:

   template <adjacency_list_graph Graph, class OuterExecutionPolicy =
           std::execution::parallel_unsequenced_policy,
           class InnerExecutionPolicy = std::execution::sequenced_policy>
   std::size_t triangle_count(const Graph& A, OuterExecutionPolicy&& outer = {},
           InnerExecutionPolicy inner = {}) {
     std::atomic<std::size_t> total_triangles = 0;
     std::for_each(outer, A.begin(), A.end(), [&](auto&& x) {
       std::size_t triangles = 0;
       for (auto &&i = x.begin(), e = x.end(); i != e; ++i) {
         triangles += nw::graph::intersection_size(i, e, A[std::get<0>(*i)], inner);
       }
       total_triangles += triangles;
     });
     return total_triangles;
   }

Alternatively, to explicitly manage concurrency and implement
asynchronous task-based parallel triangle counting algorithm,
``std::future`` and ``std::async`` can be used together as shown in .

.. code:: cpp
   :number-lines:

   template <class Op>
   std::size_t triangle_count_async(std::size_t threads, Op&& op) {
     std::vector<std::future<size_t>> futures(threads);
     for (std::size_t tid = 0; tid < threads; ++tid) {
       futures[tid] = std::async(std::launch::async, op, tid);
     }
     // Reduce the outcome ...
   }
   template <typename RandomAccessIterator>
   std::size_t triangle_count_v2(RandomAccessIterator first, 
    RandomAccessIterator last, std::size_t threads = 1) {
     return triangle_count_async(threads, [&](std::size_t tid) {
       std::size_t triangles = 0;
       for (auto i = first + tid; i < last; i += threads) {
         for (auto j = (*i).begin(), end = (*i).end(); j != end; ++j) {
         //  ...
   }} });}

Shortcomings of std Execution Policy-based Parallelization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The current ``std::execution`` policy and ``std::threads`` libraries
however lack adequate support for implementing efficient parallel graph
algorithms. Some of the most important limitations include:

-  No programmer control over workload distribution and partitioning
   among threads.

-  Lack of support for thread-safe data structures. Making the
   containers available in the standard library thread-safe with
   coarse-grained ``lock`` and ``mutex`` may severely limit the
   performance of parallel graph algorithms.

-  Harder to manage concurrency granularity.

Parallelization with Intel’s Threading Building Blocks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To circumvent these shortcomings, NWGraph leverages Intel’s Threading
Building Blocks (TBB) library. TBB provides a set of efficient
concurrent containers (hashmap, vector, and queue) that are implemented
based on fine-grained locking and lock-free techniques. For example, in
NWGraph, TBB’s concurrent vector is used to maintain the frontier list
of active vertices in each step of the :math:`\Delta`-stepping
algorithm :cite:`MEYER2003114` for computing the
single-source shortest paths.

One of the determinants of performant parallel graph algorithms is the
balanced workload distribution among threads. In particular, input
graphs with skewed degree distribution (aka power-law graphs) can
introduce severe workload imbalance in an algorithm. Without being able
to provide hint to the underlying parallel runtime, workload-agnostic
``parallel_for`` construct may distribute work associated with most of
the high-degree vertices to a select few threads. Workload imbalance may
introduce the straggler effect that can adversely affect the performance
of a parallel graph algorithm. As we will demonstrate in , for some
parallel algorithms, relabeling-by-degree (i.e. sorting vertices by
their degrees and relabeling the vertices with IDs based on their
degrees) and cyclic workload distribution techniques may significantly
improve the performance of graph algorithms with skewed graph inputs.
For providing better control for workload distribution among threads,
TBB’s ``parallel_for`` construct accepts ranges (``blocked_range``,
customized cyclic range, etc.). An example of using ``blocked_range`` in
the :math:`\Delta`-stepping algorithm is shown in . TBB also supports
user-defined custom range such as cyclic range in the ``parallel_for``
loop construct, so that better load balancing among the threads can be
achieved. It is also possible to specify the granularity of work (chunk
or block size) for each thread.

.. code:: cpp
   :number-lines:

   template <class distance_t, adjacency_list_graph Graph, class Id, class T>
   auto delta_stepping(const Graph& graph, Id source, T delta) {
     tbb::queuing_mutex                                 lock;
     tbb::concurrent_vector<tbb::concurrent_vector<Id>> bins(size);
     tbb::concurrent_vector<Id> frontier;
     // ...
     while (top_bin < bins.size()) {
       frontier.resize(0);
       std::swap(frontier, bins[top_bin]);
       tbb::parallel_for_each(frontier, [&](auto&& u) {
         if (tdist[u] >= delta * top_bin) {
           nw::graph::parallel_for(graph[u], [&](auto&& v, auto&& wt) { 
               relax(u, v, wt); });
         } });
    // ...
   }

.. code:: cpp
   :number-lines:

     // ...    
     while (top_bin < bins.size()) {
       // ...
      tbb::parallel_for(tbb::blocked_range(0ul, frontier.size()), [&](auto&& range){
         for (auto id = range.begin(), e = range.end(); id < e; ++id) {
           auto i = frontier[id];
           if (tdist[i] >= delta * top_bin) {
           // ...
   }}}});
