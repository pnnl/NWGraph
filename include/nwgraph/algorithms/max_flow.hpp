/**
 * @file max_flow.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *
 */

#ifndef NW_GRAPH_MAX_FLOW_HPP
#define NW_GRAPH_MAX_FLOW_HPP

#include "nwgraph/adaptors/back_edge_range.hpp"
#include "nwgraph/adaptors/filtered_bfs_range.hpp"
#include "nwgraph/adaptors/reverse.hpp"
#include "nwgraph/util/util.hpp"
#include <algorithm>
#include <iostream>
#include <tuple>

namespace nw {
namespace graph {

static int DEFAULT_MAX = 1000;
enum class default_dict { capacity_idx = 1, flow_idx = 2 };
/**
 * @brief A helper function to access the property in a backedge (a tuple)
 * 
 * @tparam Idx the Idx-th element in the backedge
 * @tparam Edge the type of the backedge
 * @param edge input backedge
 * @return auto the Idx-the element in the backedge
 */
template <size_t Idx, typename Edge>
auto backedge_property(Edge edge) {
  return std::get<Idx>(edge);
}

/**
 * @brief A sequetial algorithm to compute the max flow within a graph.
 * 
 * @tparam Dict the dictionary type for max flow
 * @tparam flowtype the type for flow value
 * @tparam Graph adjacency_list_graph graph type
 * @param A input graph
 * @param source source vertex
 * @param sink sink vertex
 * @param max_iters the maximum number of iterations 
 * @return flowtype the max flow value
 */
template <typename Dict = default_dict, typename flowtype = double, adjacency_list_graph Graph>
flowtype max_flow(const Graph& A, vertex_id_type source, vertex_id_type sink, size_t max_iters = DEFAULT_MAX) {
  struct tree_edge {
    flowtype* capacity;
    flowtype* flow;
    flowtype* back_flow;
    size_t    predecessor;
  };

  size_t                 n_vtx = A.size();
  std::vector<tree_edge> search_tree(n_vtx);
  flowtype               maxflow = 0;

  for (size_t i = 0; i <= max_iters; ++i) {
    auto mf_filter = [&A](vertex_id_type vtx, typename Graph::inner_iterator edge) {
      return property<idx(Dict::flow_idx)>(edge) >=
             property<idx(Dict::capacity_idx)>(edge) + backedge_property<idx(Dict::flow_idx)>(A.get_back_edge(vtx, edge));
    };

    filtered_bfs::filtered_bfs_edge_range ranges(A, source, sink, mf_filter);
    auto                                  ite = ranges.begin();
    for (; ite != ranges.end(); ++ite) {
      auto v                     = get_source(ite);
      auto u                     = get_target(ite);
      auto c                     = property_ptr<idx(Dict::capacity_idx)>(ite);
      auto f                     = property_ptr<idx(Dict::flow_idx)>(ite);
      search_tree[u].capacity    = c;
      search_tree[u].flow        = f;
      search_tree[u].predecessor = v;
      search_tree[u].back_flow   = &std::get<idx(Dict::flow_idx)>(A.get_back_edge(v, u));
    }

    if (!ite.found()) break;

    flowtype min_cap     = 100000;
    size_t   path_length = 0;
    for (auto y : reverse_path(search_tree, sink, source)) {
      min_cap = std::min(min_cap, *(search_tree[std::get<0>(y)].capacity) - *(search_tree[std::get<0>(y)].flow) +
                                      *(search_tree[std::get<0>(y)].back_flow));
      ++path_length;
    }

    maxflow += min_cap;
    for (auto y : reverse_path(search_tree, sink, source)) {
      *(search_tree[std::get<0>(y)].flow) += min_cap;
      auto excess = *(search_tree[std::get<0>(y)].flow) - *(search_tree[std::get<0>(y)].capacity);
      if (excess > 0) {
        *(search_tree[std::get<0>(y)].flow) = *(search_tree[std::get<0>(y)].capacity);
        *(search_tree[std::get<0>(y)].back_flow) -= excess;
      }
    }
  }

  return maxflow;
}
}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_MAX_FLOW_HPP
