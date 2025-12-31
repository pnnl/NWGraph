/**
 * @file ch6_prim.cpp
 *
 * @brief Prim's Minimum Spanning Tree Algorithm (BGL Book Chapter 6)
 *
 * This example demonstrates Prim's algorithm for finding a minimum spanning
 * tree of an undirected weighted graph. Unlike Kruskal's algorithm which
 * works with edges, Prim's algorithm grows the MST from a starting vertex.
 *
 * Both algorithms produce the same MST (or one of equal weight if there
 * are ties), but Prim's is often more efficient for dense graphs.
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

#include <iostream>
#include <limits>
#include <queue>
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

/**
 * @brief Simple implementation of Prim's MST algorithm
 *
 * This is a straightforward implementation similar to the BGL book's approach.
 * It uses a priority queue to always select the minimum weight edge connecting
 * a visited vertex to an unvisited vertex.
 *
 * @return Vector of predecessor vertices (the MST as a parent array)
 */
template <typename Graph, typename Weight>
auto prim_mst(const Graph& G, size_t source, Weight weight_fn) {
  size_t N = G.size();

  std::vector<double> distance(N, std::numeric_limits<double>::max());
  std::vector<size_t> predecessor(N, std::numeric_limits<size_t>::max());
  std::vector<bool> in_mst(N, false);

  distance[source] = 0;

  // Priority queue: (distance, vertex)
  using pq_element = std::pair<double, size_t>;
  std::priority_queue<pq_element, std::vector<pq_element>, std::greater<pq_element>> pq;
  pq.push({0, source});

  while (!pq.empty()) {
    auto [d, u] = pq.top();
    pq.pop();

    if (in_mst[u]) continue;
    in_mst[u] = true;

    // Examine all adjacent vertices
    for (auto&& edge : G[u]) {
      auto v = std::get<0>(edge);
      auto w = weight_fn(edge);

      // Key difference from Dijkstra: we compare edge weight, not total path
      if (!in_mst[v] && w < distance[v]) {
        distance[v] = w;
        predecessor[v] = u;
        pq.push({distance[v], v});
      }
    }
  }

  return std::make_pair(predecessor, distance);
}

int main() {
  std::cout << "=== Prim's Minimum Spanning Tree ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 6" << std::endl << std::endl;

  // Create the same graph as in ch6_kruskal.cpp for comparison
  std::cout << "Graph: 7 vertices representing cities" << std::endl;
  std::cout << "Same graph as Kruskal example for comparison" << std::endl;
  std::cout << std::endl;

  // For Prim's algorithm, we need an adjacency list (for efficient neighbor access)
  // Using undirected graph, so we add edges in both directions
  edge_list<directedness::undirected, double> edges(7);
  edges.open_for_push_back();
  edges.push_back(0, 1, 7.0);   // A-B
  edges.push_back(0, 3, 5.0);   // A-D
  edges.push_back(1, 2, 8.0);   // B-C
  edges.push_back(1, 3, 9.0);   // B-D
  edges.push_back(1, 4, 7.0);   // B-E
  edges.push_back(2, 4, 5.0);   // C-E
  edges.push_back(3, 4, 15.0);  // D-E
  edges.push_back(3, 5, 6.0);   // D-F
  edges.push_back(4, 5, 8.0);   // E-F
  edges.push_back(4, 6, 9.0);   // E-G
  edges.push_back(5, 6, 11.0);  // F-G
  edges.close_for_push_back();

  // Build symmetric adjacency list
  adjacency<0, double> G(edges);

  std::cout << "Running Prim's algorithm from vertex 0..." << std::endl;
  std::cout << std::endl;

  auto weight_fn = [](auto&& edge) { return std::get<1>(edge); };
  auto [predecessor, key] = prim_mst(G, 0, weight_fn);

  // Display MST as predecessor tree
  std::cout << "MST as predecessor array:" << std::endl;
  double total_weight = 0.0;
  for (size_t v = 0; v < G.size(); ++v) {
    if (predecessor[v] != std::numeric_limits<size_t>::max()) {
      std::cout << "  " << predecessor[v] << " -> " << v << " : " << key[v] << std::endl;
      total_weight += key[v];
    }
  }

  std::cout << std::endl;
  std::cout << "Total MST weight: " << total_weight << std::endl;
  std::cout << std::endl;

  // Show the key (distance) for each vertex
  std::cout << "Final key values (edge weight to MST):" << std::endl;
  for (size_t v = 0; v < G.size(); ++v) {
    std::cout << "  Vertex " << v << ": ";
    if (key[v] == std::numeric_limits<double>::max()) {
      std::cout << "inf (unreachable)";
    } else {
      std::cout << key[v];
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Note: Both Kruskal and Prim produce the same MST weight (39.0)" << std::endl;
  std::cout << "Prim grows the tree from a single vertex, Kruskal merges forests." << std::endl;

  return 0;
}
