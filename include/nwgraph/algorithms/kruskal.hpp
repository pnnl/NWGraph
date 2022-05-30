/**
 * @file kruskal.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *
 */

#ifndef NW_GRAPH_KRUSKAL_HPP
#define NW_GRAPH_KRUSKAL_HPP

#include <algorithm>
#include <tuple>

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/util/disjoint_set.hpp"
#include "nwgraph/util/util.hpp"
#include "nwgraph/edge_list.hpp"

#include "nwgraph/util/print_types.hpp"


namespace nw {
namespace graph {

/**
 * @brief A wrapper function to avoid pass compare function as an arg.
 * 
 * @tparam EdgeListT the edge_list_graph graph type
 * @param E input edge list
 * @return EdgeListT output edge list of the minimum spanning tree
 */
template <edge_list_graph EdgeListT>
EdgeListT kruskal(EdgeListT& E) {
  return kruskal(E, [](auto t1, auto t2) { return std::get<2>(t1) < std::get<2>(t2); });
}
/**
 * @brief A sequential Kruskal's algorithm to find a minimum spanning tree of an undirected edge-weighted graph.
 * 
 * @tparam EdgeListT the edge_list_graph graph type
 * @tparam Compare the comparison function type
 * @param E input edge list
 * @param comp comparison function object for sorting the input edge list
 * @return EdgeListT output edge list of the minimum spanning tree
 */
template <edge_list_graph EdgeListT, typename Compare>
EdgeListT kruskal(EdgeListT& E, Compare comp) {
  size_t    n_vtx = E.size();
  EdgeListT T(n_vtx);
  std::sort(E.begin(), E.end(), comp);

  std::vector<std::pair<vertex_id_type, size_t>> subsets(n_vtx);
  for (size_t i = 0; i < n_vtx; ++i) {
    subsets[i].first  = i;
    subsets[i].second = 0;
  }

  for (auto y : E) {
    //    auto u = std::get<0>(y);
    //    auto v = std::get<1>(y);

    auto u = source(E, y);
    auto v = target(E, y);
    if (disjoint_union_find(subsets, u, v)) T.push_back(y);
  }

  return T;
}

}    // namespace graph
}    // namespace nw

#endif    //  NW_GRAPH_KRUSKAL_HPP
