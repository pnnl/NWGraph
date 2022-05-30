/**
 * @file dijkstra.hpp
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


#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <algorithm>
#include <cassert>
#include <queue>
#include <tuple>
#include <vector>
#include <limits>

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/adaptors/bfs_edge_range.hpp"

namespace nw {
namespace graph {

/**
 * Basic Dijkstra's single-source shortest-paths algorithm, based on bfs_edge_range adaptor.
 *
 * @tparam Type of the edge weights (distances).
 * @tparam Graph Type of the input graph.  Must meet the requirements of the adjacency_list_graph concept.
 * @param graph The input graph.
 * @param source The starting vertex.
 * @return Vector of distances from the starting node for each vertex in the graph.
 */
template <typename Distance, adjacency_list_graph Graph>
std::vector<Distance> dijkstra_er(const Graph& graph, vertex_id_t<Graph> source) {
  using vertex_id_type = vertex_id_t<Graph>;

  size_t N(graph.end() - graph.begin());
  assert(source < N);

  std::vector<Distance> distance(N, 0xDEADBEEF);
  distance[source] = 0;

  using weight_t        = Distance;
  using weighted_vertex = std::tuple<vertex_id_type, weight_t>;

  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, std::greater<weighted_vertex>> Q;

  for (auto&& [u, v, w] : bfs_edge_range2(graph, Q)) {
    if (distance[u] + w < distance[v]) {
      distance[v] = distance[u] + w;
      Q.push({v, distance[v]});
    }
  }

  return distance;
}

/**
 * Dijkstra's single-source shortest-paths algorithm, lifted per 
 * @verbatim embed:rst:inline :ref:`Lifting Edge Weight`.@endverbatim  
 *
 * @tparam Type of the edge weights (distances).
 * @tparam Graph Type of the input graph.  Must meet the requirements of the adjacency_list_graph concept.
 * @tparam Weight Type of function used to compute edge weights.
 * @param graph The input graph.
 * @param source The starting vertex.
 * @param weight Function for computing edge weight.
 * @return Vector of distances from the starting node for each vertex in the graph.
 */
template <
    typename Distance, adjacency_list_graph Graph,
    std::invocable<inner_value_t<Graph>> Weight = std::function<std::tuple_element_t<1, inner_value_t<Graph>>(const inner_value_t<Graph>&)>>
auto dijkstra(
    const Graph& graph, vertex_id_t<Graph> source, Weight weight = [](auto& e) { return std::get<1>(e); }) {
  using vertex_id_type = vertex_id_t<Graph>;

  size_t N(graph.end() - graph.begin());
  assert(source < N);

  std::vector<Distance> distance(N, std::numeric_limits<vertex_id_type>::max());
  distance[source] = 0;

  auto g                = graph.begin();
  using weight_t        = Distance;
  using weighted_vertex = std::tuple<vertex_id_type, weight_t>;

  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, std::greater<weighted_vertex>> Q;

  Q.push({source, distance[source]});

  while (!Q.empty()) {

    auto u = std::get<0>(Q.top());
    Q.pop();

    std::for_each(g[u].begin(), g[u].end(), [&](auto&& e) {
      auto v = target(graph, e);
      auto w = weight(e);
      if (distance[u] + w < distance[v]) {
        distance[v] = distance[u] + w;
        Q.push({v, distance[v]});
      }
    });
  }
  return distance;
}

}    // namespace graph
}    // namespace nw
#endif    // DIJKSTRA_HPP
