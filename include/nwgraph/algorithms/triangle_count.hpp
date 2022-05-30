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
 *   Tony Liu
 *   Kevin Deweese
 *
 */

#ifndef NW_GRAPH_TRIANGLE_COUNT_HPP
#define NW_GRAPH_TRIANGLE_COUNT_HPP

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

/**
 * @brief Sequential 2D triangle counting algorithm set intersection the neighbor lists of each pair of vertices.
 * 
 * @tparam GraphT adjacency_list_graph
 * @param A graph
 * @return size_t the number of triangles
 */
template <adjacency_list_graph GraphT>
size_t triangle_count(const GraphT& A) {
  size_t triangles = 0;
  auto   first     = A.begin();
  auto   last      = A.end();
  for (auto u_neighors = first; u_neighors != last; ++u_neighors) {
    for (auto elt = (*u_neighors).begin(); elt != (*u_neighors).end(); ++elt) {
      auto v = target(A, *elt);
      triangles += nw::graph::intersection_size(*u_neighors, A[v]);
    }
  }
  return triangles;
}

/// Parallel triangle counting using `std::async`.
///
/// This version of triangle counting uses `threads` `std::async` launches to
/// evaluate the passed `op` in parallel. The `op` will be provided the thread
/// id, but should capture any other information required to perform the
/// decomposed work.
///
/// @tparam          Op The type of the decomposed work.
///
/// @param           op The decomposed work for each `std::async`.
///
/// @return             The += reduced total of counted triangles.
template <class Op>
std::size_t triangle_count_async(std::size_t threads, Op&& op) {
  // Launch the workers.
  std::vector<std::future<size_t>> futures(threads);
  for (std::size_t tid = 0; tid < threads; ++tid) {
    futures[tid] = std::async(std::launch::async, op, tid);
  }

  // Reduce the outcome.
  int         i         = 0;
  std::size_t triangles = 0;
  for (auto&& f : futures) {
    triangles += f.get();
  }
  return triangles;
}


/**
 * @brief Two-dimensional triangle counting, parallel version.
 * 
 * @tparam Graph adjacency_list_graph
 * @param G graph
 * @param threads number of threads
 * @return std::size_t number of triangles
 */
template <adjacency_list_graph Graph>
[[gnu::noinline]] std::size_t triangle_count(const Graph& G, std::size_t threads) {
  auto first = G.begin();
  auto last = G.end();
  return triangle_count_async(threads, [&](std::size_t tid) {
    std::size_t triangles = 0;
    for (auto i = first + tid; i < last; i += threads) {
      for (auto j = (*i).begin(), end = (*i).end(); j != end; ++j) {
        // assert(j < end);
        triangles += nw::graph::intersection_size(j, end, first[target(G, *j)]);
      }
    }
    return triangles;
  });
}

}    // namespace graph
}    // namespace nw

#endif    //  NW_GRAPH_TRIANGLE_COUNT_HPP
