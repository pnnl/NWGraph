/**
 * @file prim.hpp
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

#ifndef NW_GRAPH_PRIM_HPP
#define NW_GRAPH_PRIM_HPP

#include "nwgraph/edge_list.hpp"
#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/graph_traits.hpp"

#include <vector>

namespace nw {
namespace graph {

/**
 * @brief Prim's minimum spanning tree algorithm
 * A greedy algorithm for a weighted undirected graph
 *
 * @see Cormen, Leiserson, Rivest, and Stein, "Introduction to Algorithms",
 *      4th Edition (2022), Chapter 21.2: Kruskal's and Prim's Algorithms
 *
 * @tparam Graph Graph type.  Must meet requirements of adjacency_list_graph.
 * @tparam Distance Type of edge weight.
 * @tparam Weight A weight function for a given edge, returns a Distance.
 * @param graph Input graph.
 * @param source Starting vertex.
 * @param weight Weight function that returns the edge weight.
 * @return std::vector<vertex_id_t<Graph>>, the predecessor list (the MST).
 */
template <adjacency_list_graph Graph, class Distance, class Weight>
std::vector<vertex_id_t<Graph>> prim(const Graph& graph, vertex_id_t<Graph> source, Weight&& weight) {

  using vertex_id_type = vertex_id_t<Graph>;

  size_t N { num_vertices(graph) };
  assert(source < N);

  std::vector<Distance>       distance(N, std::numeric_limits<Distance>::max());
  std::vector<vertex_id_type> predecessor(N, std::numeric_limits<vertex_id_type>::max());
  std::vector<uint8_t>  finished(N, false);
  distance[source] = 0;

  using weight_t        = Distance;
  using weighted_vertex = std::tuple<vertex_id_type, weight_t>;

  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, std::greater<weighted_vertex>> Q;

  Q.push({source, 0});

  while (!Q.empty()) {

    auto u = std::get<0>(Q.top());
    Q.pop();

    if (finished[u]) {
      continue;
    }
    finished[u] = true;

    std::for_each(graph[u].begin(), graph[u].end(), [&](auto&& e) {
      auto v = target(graph, e);
      auto w = weight(e);

      if (!finished[v] && distance[v] > w) {
        distance[v] = w;
        Q.push({ v, distance[v] });
        predecessor[v] = u;
      }
    });
  }

  return predecessor;
}

}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_PRIM_HPP
