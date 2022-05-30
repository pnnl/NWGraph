/**
 * @file triangle_count.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese	
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_TRIANGLE_COUNT_EXPERIMENTAL_HPP
#define NW_GRAPH_TRIANGLE_COUNT_EXPERIMENTAL_HPP

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/adaptors/cyclic_range_adaptor.hpp"
#include "nwgraph/adaptors/neighbor_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/util/intersection_size.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/util/timer.hpp"
#include "nwgraph/util/util.hpp"

#include <atomic>
#include <future>
#include <thread>
#include <tuple>
#include <vector>

namespace nw {
namespace graph {

template <typename _InputIterator1, typename _InputIterator2, typename _OutputIterator>
_OutputIterator lower_bound_set_intersection(_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2,
                                             _InputIterator2 __last2, _OutputIterator __result) {
  while (__first1 != __last1 && __first2 != __last2)
    if (*__first1 < *__first2) {
      ++__first1;
      __first1 = std::lower_bound(__first1, __last1, *__first2);
    } else if (*__first2 < *__first1) {
      ++__first2;
      __first2 = std::lower_bound(__first2, __last2, *__first1);
    } else {
      *__result = *__first1;
      ++__first1;
      ++__first2;
      ++__result;
    }
  return __result;
}

template <adjacency_list_graph GraphT>
size_t triangle_count_vc(const GraphT& A) {
  size_t                   triangles = 0;
  counting_output_iterator counter(triangles);

  size_t len  = A.indices_.size() - 1;
  auto   ptrs = A.indices_;
  auto   idxs = std::get<0>(A.to_be_indexed_);

  for (size_t i = 0; i < len; ++i) {
    for (size_t j = ptrs[i]; j < ptrs[i + 1]; ++j) {
      size_t k  = idxs[j];
      size_t k0 = ptrs[i];
      size_t k1 = ptrs[i + 1];
      size_t k2 = ptrs[k];
      size_t k3 = ptrs[k + 1];

      lower_bound_set_intersection(&idxs[k2], &idxs[k3], &idxs[k0], &idxs[k1], counter);
    }
  }

  return triangles;
}


template <adjacency_list_graph GraphT>
size_t triangle_count_v1(const GraphT& A) {
  size_t triangles = 0;
  auto   first     = A.begin();
  auto   last      = A.end();

  for (auto G = first; first != last; ++first) {
    for (auto elt = (*first).begin(); elt != (*first).end(); ++elt) {
      auto v = target(A, *elt);
      triangles += nw::graph::intersection_size(elt, (*first).end(), G[v]);
    }
  }
  return triangles;
}

template <adjacency_list_graph GraphT>
size_t triangle_count_v15(const GraphT& A) {
  size_t triangles = 0;

  for (auto&& [u, u_neighbors] : make_neighbor_range(A)) {
    for (auto&& elt : u_neighbors) {
      auto v = target(A, elt);
      triangles += nw::graph::intersection_size(u_neighbors, A[v]);
    }
  }
  return triangles;
}

//****************************************************************************
// Should this work?
template <adjacency_list_graph GraphT>
size_t triangle_count_v2(const GraphT& A) {
  size_t triangles = 0;
  size_t i         = 0;
  auto   G         = A.begin();

  for (auto&& node : A) {
    for (auto&& elt : node) {
      auto u = target(A, elt);
      triangles += nw::graph::intersection_size(G[u], G[i]);
    }
    ++i;
  }
  return triangles;
}

//****************************************************************************
template <class Graph>
size_t triangle_count_v3(const Graph& A) {
  size_t triangles = 0;
  for (auto&& [v, u] : make_edge_range(A)) {
    triangles += nw::graph::intersection_size(A[v], A[u]);
  }
  return triangles;
}
/// Two-dimensional triangle counting.
///
/// This version of triangle counting is explicitly two-dimensional and is
/// optimized (and only correct for) an upper-triangular graph. It uses explicit
/// async threads to perform the parallelization.
///
/// @tparam RandomAccessIterator A random access BGL17 iterator type.
///
/// @param        first The beginning of the outer range.
/// @param         last The end of the outer range.
/// @param      threads The number of threads to use in the parallelization.
///
/// @returns            The number of triangles in the graph.
template <typename RandomAccessIterator>
[[gnu::noinline]] std::size_t triangle_count_v4(RandomAccessIterator first, RandomAccessIterator last, std::size_t threads = 1) {
  return triangle_count_async(threads, [&](std::size_t tid) {
    std::size_t triangles = 0;
    for (auto i = first + tid; i < last; i += threads) {
      for (auto j = (*i).begin(), end = (*i).end(); j != end; ++j) {
        // assert(j < end);
        triangles += nw::graph::intersection_size(j, end, first[std::get<0>(*j)]);
      }
    }
    return triangles;
  });
}

/// Two-dimensional triangle counting.
///
/// This version of triangle counting is explicitly two-dimensional and is
/// optimized (and only correct for) an upper-triangular graph. It uses a
/// blocked work distribution and uses explicit async threads for
/// parallelization.
///
/// @tparam RandomAccessIterator A random access iterator type.
///
/// @param        first The beginning of the outer range.
/// @param         last The end of the outer range.
/// @param      threads The number of threads to use in the parallelization.
///
/// @returns            The number of triangles in the graph.
template <typename RandomAccessIterator>
[[gnu::noinline]] std::size_t triangle_count_v5(RandomAccessIterator first, RandomAccessIterator last, std::size_t threads = 1) {
  return triangle_count_async(threads, [&](std::size_t tid) {
    std::size_t triangles = 0;
    auto&& [u, ue]        = nw::graph::block(last - first, threads, tid);
    for (auto i = first + u, end = first + ue; i != end; ++i) {
      for (auto j = (*i).begin(), end = (*i).end(); j != end; ++j) {
        triangles += nw::graph::intersection_size(j, end, first[std::get<0>(*j)]);
      }
    }
    return triangles;
  });
}

/// Two-dimensional triangle counting.
///
/// This version of triangle counting is explicitly two-dimensional. It works on
/// graphs that are either upper or lower triangular. It uses explicit async
/// threads to perform the parallelization.
///
/// @tparam RandomAccessIterator A random access BGL17 iterator type.
///
/// @param        first The beginning of the outer range.
/// @param         last The end of the outer range.
/// @param      threads The number of threads to use in the parallelization.
///
/// @returns            The number of triangles in the graph.
template <typename RandomAccessIterator>
[[gnu::noinline]] std::size_t triangle_count_v6(RandomAccessIterator first, RandomAccessIterator last, std::size_t threads = 1) {
  return triangle_count_async(threads, [&](std::size_t tid) {
    std::size_t triangles = 0;
    for (auto&& i = first + tid; i < last; i += threads) {
      for (auto&& [j] : *i) {
        triangles += nw::graph::intersection_size(*i, first[j]);
      }
    }
    return triangles;
  });
}

/// Two-dimensional triangle counting.
///
/// This version of triangle counting is explicitly two-dimensional. It works on
/// graphs that are upper triangular, and uses a `std::for_each` for the outer
/// loop decomposition.
///
/// This version takes an outer and inner execution policy. The outer policy is
/// used to parallelize the `std::for_each` while the inner policy is used to
/// parallelize the `std::set_intersection`. By default the `std::for_each` is
/// parallel while the `std::set_intersection` is sequential.
///
/// @tparam       Graph The graph type (random-access range of ranges).
/// @tparam OuterExecutionPolicy The parallel execution policy for the
///                     `std::for_each` [default: `parallel_unsequenced_policy`]
/// @tparam InnerExecutionPolicy The parallel execution policy for the
///                     `std::set_intersection` [default: `sequenced_policy`].
///
/// @param            A The graph *(must be upper triangular)*.
/// @param        outer The outer execution policy.
/// @param        inner The inner execution policy.
///
/// @returns            The number of triangles in the graph.
template <adjacency_list_graph Graph, class OuterExecutionPolicy = std::execution::parallel_unsequenced_policy,
          class InnerExecutionPolicy = std::execution::sequenced_policy>
[[gnu::noinline]] std::size_t triangle_count_v7(const Graph& A, OuterExecutionPolicy&& outer = {}, InnerExecutionPolicy inner = {}) {
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

/// Two-dimensional triangle counting.
///
/// This version of triangle counting is explicitly two dimensional, uses basic
/// `std::for_each` loop parallelism for the two loops, and supports a parallel
/// inner set intersection. It works on either upper or lower triangular
/// graphs.
///
/// @tparam       Graph The graph type (random-access range of ranges).
/// @tparam OuterExecutionPolicy The parallel execution policy for the outer
///                     `std::for_each` [default: `parallel_unsequenced_policy`]
/// @tparam InnerExecutionPolicy The parallel execution policy for the inner
///                     `std::for_each` [default: `parallel_policy`]
/// @tparam SetExecutionPolicy The parallel execution policy for the
///                     `std::set_intersection` [default: `sequenced_policy`].
///
/// @param            A The graph *(must be upper triangular)*.
/// @param        outer The outer execution policy.
/// @param        inner The inner execution policy.
/// @param          set The set intersection execution policy.
///
/// @returns            The number of triangles in the graph.
template <adjacency_list_graph Graph, class OuterExecutionPolicy = std::execution::parallel_unsequenced_policy,
          class InnerExecutionPolicy = std::execution::parallel_policy, class SetExecutionPolicy = std::execution::sequenced_policy>
[[gnu::noinline]] std::size_t triangle_count_v10(const Graph& A, OuterExecutionPolicy&& outer = {}, InnerExecutionPolicy&& inner = {},
                                                 SetExecutionPolicy&& set = {}) {
  std::atomic<std::size_t> total_triangles = 0;
  std::for_each(outer, A.begin(), A.end(), [&](auto&& x) {
    std::atomic<std::size_t> triangles = 0;
    std::for_each(inner, x.begin(), x.end(), [&](auto&& v) { triangles += nw::graph::intersection_size(x, A[std::get<0>(v)], set); });
    total_triangles += triangles;
  });
  return total_triangles;
}

/// Two-dimensional triangle counting.
///
/// This version of triangle counting is explicitly two dimensional, is
/// optimized (and only correct for) an upper-triangular graph, and uses the
/// cyclic range adapter and a `parallel for` to process the outer dimension.
///
/// @precondition       The graph must be upper-triangular for this to work
///                     properly.
///
/// @tparam       Graph The graph type (a random-access range of ranges).
/// @tparam SetExecutionPolicy The parallel execution policy for the
///                     `std::set_intersection` [default: `sequenced_policy`].
///
/// @param        graph The graph *(must be upper-triangular)*.
/// @param       stride The maximum stride for the cyclic outer loop.
/// @param          set The execution policy for the set intersection.
///
/// @returns            The number of triangles in the graph.
template <adjacency_list_graph Graph, class SetExecutionPolicy = std::execution::sequenced_policy>
[[gnu::noinline]] std::size_t triangle_count_v12(const Graph& graph, int stride, SetExecutionPolicy&& set = {}) {
  return nw::graph::parallel_reduce(
      nw::graph::cyclic(graph, stride),
      [&](auto&& i) {
        std::size_t triangles = 0;
        for (auto &&j = i.begin(), e = i.end(); j != e; ++j) {
          triangles += nw::graph::intersection_size(j, e, graph[std::get<0>(*j)], set);
        }
        return triangles;
      },
      std::plus{}, 0.0);
}

/// Two-dimensional triangle counting.
///
/// This version of triangle counting is explicitly two dimensional and uses the
/// cyclic range adapter and a `parallel for` to process the outer dimension.
///
/// @note               This implementation is safe for both upper and lower
///                     triangular matrices, but should be more efficient for
///                     lower triangular matrices as the inner intersections
///                     will be smaller.
///
/// @tparam       Graph The graph type (a random-access range of ranges).
/// @tparam SetExecutionPolicy The parallel execution policy for the
///                     `std::set_intersection` [default: `sequenced_policy`].
///
/// @param        graph The graph.
/// @param       stride The maximum stride for the cyclic outer loop.
/// @param          set The execution policy for the set intersection.
///
/// @returns            The number of triangles in the graph.
template <adjacency_list_graph Graph, class SetExecutionPolicy = std::execution::sequenced_policy>
[[gnu::noinline]] std::size_t triangle_count_v13(const Graph& graph, int stride, SetExecutionPolicy&& set = {}) {
  return nw::graph::parallel_reduce(
      nw::graph::cyclic(graph, stride),
      [&](auto&& i) {
        std::size_t triangles = 0;
        for (auto&& elt : i) {
          auto j = target(graph, elt);
          triangles += nw::graph::intersection_size(i, graph[j], set);
        }
        return triangles;
      },
      std::plus{}, 0.0);
}

/// One-dimensional triangle counting.
///
/// This version of triangle counting uses a one dimensional parallel-for loop
/// over the edge range. This is identical to the the GAP verification code, but
/// in parallel.
///
/// @note               This version produces the correct result for both upper
///                     and lower triangular matrices, but should be somewhat
///                     more efficient for lower triangular orders as the
///                     set intersections should be more efficient.
///
/// @tparam       Graph The graph type (a random-access range of ranges).
/// @tparam SetExecutionPolicy The parallel execution policy for the
///                     `std::set_intersection` [default: `sequenced_policy`].
///
/// @param        graph The graph.
/// @param          set The execution policy for the set intersection.
///
/// @return             The number of triangles in the graph.
template <adjacency_list_graph Graph, class SetExecutionPolicy = std::execution::sequenced_policy>
[[gnu::noinline]] std::size_t triangle_count_v14(const Graph& graph, SetExecutionPolicy&& set = {}) {
  return nw::graph::parallel_reduce(
      make_edge_range(graph), [&](auto&& u, auto&& v) { return nw::graph::intersection_size(graph[u], graph[v], set); }, std::plus{}, 0ul);
}

#ifdef ONE_DIMENSIONAL_EDGE
/// One dimensional triangle counting with an edge range.
///
/// This version of triangle counting assumes that the graph supports direct
/// edge enumeration with an edge range that supports tbb's split interface.
///
/// @tparam       Graph The graph type (a random-access range of ranges).
/// @tparam SetExecutionPolicy The parallel execution policy for the
///                     `std::set_intersection` [default: `sequenced_policy`].
///
/// @param        graph The graph.
/// @param          set The execution policy for the set intersection.
///
/// @return             The number of triangles in the graph.
template <class Graph, class SetExecutionPolicy = std::execution::sequenced_policy>
[[gnu::noinline]] std::size_t triangle_count_edgerange(const Graph& graph, SetExecutionPolicy&& set = {}) {
  return nw::graph::parallel_for(
      graph.edges(nw::graph::pow2(20)), [&](auto&& u, auto&& v) { return nw::graph::intersection_size(graph[u], graph[v], set); }, std::plus{},
      0ul);
}

/// One dimensional triangle counting with a cyclic edge range.
///
/// This version of triangle counting assumes that the graph supports direct
/// edge enumeration with an edge range that has random-access iterators and
/// supports tbb's split interface.
///
/// @tparam       Graph The graph type (a random-access range of ranges).
/// @tparam SetExecutionPolicy The parallel execution policy for the
///                     `std::set_intersection` [default: `sequenced_policy`].
///
/// @param        graph The graph.
/// @param       stride The maximum stride for the cyclic outer loop.
/// @param          set The execution policy for the set intersection.
///
/// @return             The number of triangles in the graph.
template <class Graph, class SetExecutionPolicy = std::execution::sequenced_policy>
[[gnu::noinline]] std::size_t triangle_count_edgerange_cyclic(const Graph& graph, int stride, SetExecutionPolicy&& set = {}) {
  return nw::graph::parallel_for(
      nw::graph::cyclic(graph.edges(), stride), [&](auto&& u, auto&& v) { return nw::graph::intersection_size(graph[u], graph[v], set); },
      std::plus{}, 0ul);
}
#endif

/// Two-dimensional triangle counting.
///
/// This version of triangle counting is explicitly two-dimensional and is
/// optimized (and only correct for) an upper-triangular graph. It uses explicit
/// async threads to perform the parallelization.
///
/// This parallelization uses a blocked decomposition combined with a uniform
/// edgesplit policy, where each block is assigned about the same number of
/// edges.
///
/// @tparam       Graph The graph type (a random-access range of ranges);
///
/// @param        graph The graph.
/// @param      threads The number of threads to use in the parallelization.
///
/// @returns            The number of triangles in the graph.
template <adjacency_list_graph Graph>
[[gnu::noinline]] std::size_t triangle_count_edgesplit_upper(const Graph& graph, std::size_t threads = 1) {
  return triangle_count_async(threads, [&](std::size_t tid) {
    std::size_t triangles = 0;
    auto&& [v, ve]        = nw::graph::block(graph.to_be_indexed_.size(), threads, tid);
    nw::util::life_timer _(std::to_string(tid) + " " + std::to_string(graph.source(ve) - graph.source(v)));
    for (auto &&u = graph.source(v), e = graph.source(ve); u != e; ++u) {
      for (auto &&j = graph[u].begin(), end = graph[u].end(); j != end; ++j) {
        triangles += nw::graph::intersection_size(j, end, graph[std::get<0>(*j)]);
      }
    }
    return triangles;
  });
}

template <adjacency_list_graph Graph>
[[gnu::noinline]] std::size_t triangle_count_edgesplit(const Graph& graph, std::size_t threads = 1) {
  return triangle_count_async(threads, [&](std::size_t tid) {
    std::size_t triangles = 0;
    auto&& [v, ve]        = nw::graph::block(graph.to_be_indexed_.size(), threads, tid);
    nw::util::life_timer _(std::to_string(tid) + " " + std::to_string(graph.source(ve) - graph.source(v)));
    for (auto &&u = graph.source(v), e = graph.source(ve); u != e; ++u) {
      for (auto&& elt : graph[u]) {
        auto v = target(graph, elt);
        triangles += nw::graph::intersection_size(graph[u], graph[v]);
      }
    }
    return triangles;
  });
}
}    // namespace graph
}    // namespace nw

#endif    //  NW_GRAPH_TRIANGLE_COUNT_EXPERIMENTAL_HPP
