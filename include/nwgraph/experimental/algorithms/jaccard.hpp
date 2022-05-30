/**
 * @file jaccard.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Tony Liu
 *   Andrew Lumsdaine
 *
 */

#ifndef NW_GRAPH_EXPERIMENTAL_JACCARD_HPP
#define NW_GRAPH_EXPERIMENTAL_JACCARD_HPP

#include "nwgraph/adaptors/cyclic_range_adapter.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include <atomic>
#include <future>
#include <thread>

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/util/intersection_size.hpp"
#include "nwgraph/util/timer.hpp"
#include "nwgraph/util/util.hpp"

#include <tuple>
#include <vector>

namespace nw {
namespace graph {

/**
 * @brief A sequential jaccard similarity algorithm using set intersection.
 * This version assumes each edge is a std::tuple<size_t, size_t, double>.
 * 
 * @tparam GraphT Type of graph.  Must meet the requirements of adjacency_list_graph concept.
 * @param G Input graph.
 * @return size_t Jaccard similarity score.
 */
template <typename GraphT>
size_t jaccard_similarity_v1(const GraphT& A) {
  size_t ctr = 0;
  auto   first     = A.begin();
  auto   last      = A.end();

  for (auto G = first; first != last; ++first) {
    for (auto v = (*first).begin(); v != (*first).end(); ++v) {
      auto numer = nw::graph::intersection_size(v (*first).end(), G[std::get<0>(*v)]);
      auto denom = degree(*first) + degree(G[std::get<0>(*v)]) - numer;
      double rat   = ((double)numer) / ((double)denom);
      std::get<2>(*v) = rat;
      ++ctr;
    }
  }
  return ctr;
}

#if 0
template <typename GraphT>
auto jaccard_similarity_v2(GraphT& graph) {

  size_t ctr = 0;
  auto deg = degrees(graph);
  for (auto&& [u, v, w] : make_edge_range<0, 1, 2>(graph)) {
    auto   numer = intersection_size(graph[u], graph[v]);
    auto   denom = deg[u] + deg[v] - numer;
    double rat   = ((double)numer) / ((double)denom);
    w            = rat;
    ++ctr;
  }
  return ctr;
}
#endif


}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_EXPERIMENTAL_JACCARD_HPP
