/**
 * @file jaccard.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_JACCARD_HPP
#define NW_GRAPH_JACCARD_HPP

#include <atomic>
#include <future>
#include <thread>
#include <tuple>
#include <vector>

#include "nwgraph/graph_concepts.hpp"

#include "nwgraph/adaptors/cyclic_range_adaptor.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/util/intersection_size.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/util/timer.hpp"
#include "nwgraph/util/util.hpp"

namespace nw {
namespace graph {

/**
 * @brief A sequential jaccard similarity algorithm using set intersection.
 * 
 * @tparam GraphT Type of graph.  Must meet the requirements of adjacency_list_graph concept.
 * @tparam Weight Type of the edge weight function.
 * @param G Input graph.
 * @param weight Weight function on how to access the edge weight.
 * @return size_t Jaccard similarity score.
 */
template <adjacency_list_graph GraphT, typename Weight>
size_t jaccard_similarity(GraphT& G, Weight weight) {
  size_t ctr = 0;

  for (size_t u = 0; u < num_vertices(G); ++u) {
    for (auto&& e : G[u]) {
      auto v = target(G, e);
      if (u < v) {
        auto numer = nw::graph::intersection_size(G[u], G[v]);
        auto denom = degree(G[u]) + degree(G[v]) - numer;
        double rat = ((double)numer) / ((double)denom);
        weight(e) = rat;
        ++ctr;
      }
    }
  }

  return ctr;
}

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_JACCARD_HPP
