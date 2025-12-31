/**
 * @file ch5_bellman_ford.cpp
 *
 * @brief Bellman-Ford Algorithm and Distance Vector Routing (BGL Book Chapter 5.3)
 *
 * This example demonstrates the Bellman-Ford single-source shortest-paths algorithm
 * applied to a router network. The Bellman-Ford algorithm can handle graphs with
 * negative edge weights (unlike Dijkstra) and can detect negative cycles.
 *
 * The algorithm is the basis for distance-vector routing protocols like RIP
 * (Routing Information Protocol), where routers periodically exchange distance
 * estimates with neighbors.
 *
 * Key differences from Dijkstra:
 * - Can handle negative edge weights
 * - Slower: O(V*E) vs O((V+E)log V)
 * - Can detect negative cycles
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
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

/**
 * @brief Bellman-Ford shortest paths algorithm
 *
 * Computes shortest paths from a source vertex to all other vertices.
 * Returns false if a negative cycle is detected, true otherwise.
 *
 * The algorithm works by repeatedly relaxing all edges |V|-1 times.
 * After this, all shortest paths are found (if no negative cycles exist).
 * A final pass checks for negative cycles.
 */
template <typename EdgeList, typename Weight>
auto bellman_ford(const EdgeList& edges, size_t num_vertices, size_t source, Weight weight_fn)
    -> std::tuple<bool, std::vector<double>, std::vector<size_t>> {

  std::vector<double> distance(num_vertices, std::numeric_limits<double>::max());
  std::vector<size_t> predecessor(num_vertices);

  // Initialize predecessor to self (no predecessor)
  for (size_t i = 0; i < num_vertices; ++i) {
    predecessor[i] = i;
  }

  distance[source] = 0;

  // Relax all edges |V|-1 times
  for (size_t i = 0; i < num_vertices - 1; ++i) {
    bool changed = false;
    for (auto&& edge : edges) {
      auto u = std::get<0>(edge);
      auto v = std::get<1>(edge);
      auto w = weight_fn(edge);

      // Relaxation step
      if (distance[u] != std::numeric_limits<double>::max() && distance[u] + w < distance[v]) {
        distance[v]    = distance[u] + w;
        predecessor[v] = u;
        changed        = true;
      }
    }
    // Early termination if no changes
    if (!changed) break;
  }

  // Check for negative cycles
  for (auto&& edge : edges) {
    auto u = std::get<0>(edge);
    auto v = std::get<1>(edge);
    auto w = weight_fn(edge);

    if (distance[u] != std::numeric_limits<double>::max() && distance[u] + w < distance[v]) {
      // Negative cycle detected
      return {false, distance, predecessor};
    }
  }

  return {true, distance, predecessor};
}

int main() {
  std::cout << "=== Bellman-Ford Algorithm and Distance Vector Routing ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 5.3" << std::endl << std::endl;

  // Router network from the BGL book example
  // Vertices represent routers: A=0, B=1, C=2, D=3, E=4, F=5, G=6, H=7
  // Edge weights represent transmission delays
  const char* router_names = "ABCDEFGH";
  const size_t num_routers = 8;

  std::cout << "Router network with transmission delays:" << std::endl;
  std::cout << "  A connects to B (5.0) and C (1.0)" << std::endl;
  std::cout << "  B connects to D (1.3) and E (3.0)" << std::endl;
  std::cout << "  C connects to E (10.0) and F (2.0)" << std::endl;
  std::cout << "  D connects to E (0.4) and H (6.3)" << std::endl;
  std::cout << "  E connects to H (1.3)" << std::endl;
  std::cout << "  F connects to G (1.2)" << std::endl;
  std::cout << "  G connects to H (0.5)" << std::endl;
  std::cout << std::endl;

  // Create edge list with transmission delays
  // Using directed edges as in the BGL book example
  edge_list<directedness::directed, double> edges(num_routers);
  edges.open_for_push_back();

  // Edges from the BGL book Figure 5.2
  edges.push_back(0, 1, 5.0);   // A -> B
  edges.push_back(0, 2, 1.0);   // A -> C
  edges.push_back(1, 3, 1.3);   // B -> D
  edges.push_back(1, 4, 3.0);   // B -> E
  edges.push_back(2, 4, 10.0);  // C -> E
  edges.push_back(2, 5, 2.0);   // C -> F
  edges.push_back(3, 7, 6.3);   // D -> H
  edges.push_back(3, 4, 0.4);   // D -> E
  edges.push_back(4, 7, 1.3);   // E -> H
  edges.push_back(5, 6, 1.2);   // F -> G
  edges.push_back(6, 7, 0.5);   // G -> H

  edges.close_for_push_back();

  // Run Bellman-Ford from router A (vertex 0)
  size_t source = 0;  // Router A
  std::cout << "Computing shortest paths from router " << router_names[source] << "..." << std::endl;
  std::cout << std::endl;

  auto weight_fn = [](auto&& edge) { return std::get<2>(edge); };
  auto [success, distance, predecessor] = bellman_ford(edges, num_routers, source, weight_fn);

  if (success) {
    std::cout << "Shortest path distances and predecessors:" << std::endl;
    std::cout << std::string(40, '-') << std::endl;

    for (size_t i = 0; i < num_routers; ++i) {
      std::cout << "  " << router_names[i] << ": ";
      if (distance[i] == std::numeric_limits<double>::max()) {
        std::cout << "unreachable";
      } else {
        std::cout << distance[i] << "  (via " << router_names[predecessor[i]] << ")";
      }
      std::cout << std::endl;
    }

    // Trace path to H
    std::cout << std::endl;
    std::cout << "Shortest path from A to H:" << std::endl;
    std::cout << "  ";

    std::vector<size_t> path;
    size_t current = 7;  // H
    while (current != source) {
      path.push_back(current);
      current = predecessor[current];
    }
    path.push_back(source);

    for (auto it = path.rbegin(); it != path.rend(); ++it) {
      std::cout << router_names[*it];
      if (it + 1 != path.rend()) std::cout << " -> ";
    }
    std::cout << std::endl;
    std::cout << "  Total delay: " << distance[7] << std::endl;

  } else {
    std::cout << "Negative cycle detected in the network!" << std::endl;
  }

  // Example with a negative cycle
  std::cout << std::endl;
  std::cout << "=== Example with Negative Cycle ===" << std::endl;

  edge_list<directedness::directed, double> edges2(4);
  edges2.open_for_push_back();
  edges2.push_back(0, 1, 1.0);
  edges2.push_back(1, 2, -1.0);
  edges2.push_back(2, 3, -1.0);
  edges2.push_back(3, 1, -1.0);  // Creates negative cycle: 1 -> 2 -> 3 -> 1 (cost = -3)
  edges2.close_for_push_back();

  auto [success2, distance2, predecessor2] = bellman_ford(edges2, 4, 0, weight_fn);

  if (success2) {
    std::cout << "No negative cycle found." << std::endl;
  } else {
    std::cout << "Negative cycle detected! (as expected)" << std::endl;
    std::cout << "The cycle 1 -> 2 -> 3 -> 1 has total weight -3" << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Note: Bellman-Ford is O(V*E), slower than Dijkstra's O((V+E)log V)," << std::endl;
  std::cout << "but can handle negative weights and detect negative cycles." << std::endl;

  return 0;
}
