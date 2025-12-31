/**
 * @file ch5_dijkstra.cpp
 *
 * @brief Internet Routing with Dijkstra's Algorithm (BGL Book Chapter 5.4)
 *
 * This example demonstrates Dijkstra's single-source shortest-paths algorithm
 * using an OSPF (Open Shortest Path First) router network graph. OSPF is a
 * link-state routing protocol where each router maintains a complete topology
 * of the network and computes shortest paths using Dijkstra's algorithm.
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
#include "nwgraph/algorithms/dijkstra.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/graphs/ospf-graph.hpp"

using namespace nw::graph;

int main() {
  std::cout << "=== Internet Routing with Dijkstra's Algorithm ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 5.4" << std::endl << std::endl;

  // Build the graph from the OSPF edge list data
  // The OSPF graph represents a network topology with routers (RT) and networks (N)
  size_t num_vertices = ospf_vertices.size();

  std::cout << "Network topology has " << num_vertices << " nodes:" << std::endl;
  std::cout << "  Routers: RT1-RT12" << std::endl;
  std::cout << "  Networks: N1-N15" << std::endl;
  std::cout << "  Host: H1" << std::endl << std::endl;

  // Create edge list from the OSPF index edge list
  edge_list<directedness::directed, size_t> edges(num_vertices);
  edges.open_for_push_back();
  for (auto&& [u, v, w] : ospf_index_edge_list) {
    edges.push_back(u, v, w);
  }
  edges.close_for_push_back();

  // Build adjacency representation
  adjacency<0, size_t> G(edges);

  // Run Dijkstra from RT6 (vertex 5) - this is what the BGL book example uses
  size_t source = 5;  // RT6
  std::cout << "Computing shortest paths from " << ospf_vertices[source] << "..." << std::endl;
  std::cout << std::endl;

  auto distance = dijkstra<size_t>(G, source);

  // Display results
  std::cout << "Shortest path distances from " << ospf_vertices[source] << ":" << std::endl;
  std::cout << std::string(50, '-') << std::endl;

  for (size_t i = 0; i < num_vertices; ++i) {
    std::cout << "  " << ospf_vertices[i] << ": ";
    if (distance[i] == std::numeric_limits<size_t>::max()) {
      std::cout << "unreachable";
    } else {
      std::cout << distance[i];
    }

    // Compare with expected values from the BGL book
    size_t expected = std::get<1>(ospf_shortest_path_distances[i]);
    if (distance[i] != expected && distance[i] != std::numeric_limits<size_t>::max()) {
      std::cout << " (expected: " << expected << ")";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Note: In OSPF, link costs are typically based on bandwidth." << std::endl;
  std::cout << "Routers use these shortest paths to build their routing tables." << std::endl;

  return 0;
}
