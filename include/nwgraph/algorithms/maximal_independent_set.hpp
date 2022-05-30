/**
 * @file mis.hpp
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

#ifndef NW_GRAPH_MIS_HPP
#define NW_GRAPH_MIS_HPP

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/adaptors/bfs_range.hpp"

namespace nw {
namespace graph {

/**
 * @brief A sequential algorithm to find maximal independent set in a graph.
 * 
 * @tparam Graph input adjacency_list_graph type
 * @param A input graph
 * @param mis the vertices in the maximal independent set
 */
template <adjacency_list_graph Graph>
void maximal_independent_set(const Graph& A, std::vector<size_t>& mis) {
  using vertex_id_type = vertex_id_t<Graph>;
  size_t            N = A.size();
  std::vector<bool> removedVertices(N);
  for (vertex_id_type vtx = 0; vtx < N; vtx++) {
    if (!removedVertices[vtx]) {
      mis.push_back(vtx);
      for (auto ite = A[vtx].begin(); ite != A[vtx].end(); ++ite) {
        // Explore neighbors
        auto u = target(A, *ite);
        removedVertices[u] = true;
      }
    }
  }
}
}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_MIS_HPP
