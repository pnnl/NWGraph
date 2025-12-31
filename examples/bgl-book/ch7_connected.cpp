/**
 * @file ch7_connected.cpp
 *
 * @brief Connected Components (BGL Book Chapter 7)
 *
 * This example demonstrates finding connected components in a graph.
 * A connected component is a maximal subgraph where every vertex is
 * reachable from every other vertex.
 *
 * Application: Network analysis, image segmentation, social network clusters.
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

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

/**
 * @brief Simple BFS-based connected components algorithm
 *
 * This implementation uses BFS to explore each component, similar to
 * the approach in the BGL book.
 */
template <typename Graph>
std::vector<size_t> compute_connected_components_bfs(const Graph& G) {
  size_t N = G.size();
  std::vector<size_t> component(N, std::numeric_limits<size_t>::max());
  size_t current_component = 0;

  for (size_t start = 0; start < N; ++start) {
    if (component[start] != std::numeric_limits<size_t>::max()) {
      continue;  // Already assigned to a component
    }

    // BFS from this vertex
    component[start] = current_component;
    std::vector<size_t> frontier;
    frontier.push_back(start);

    while (!frontier.empty()) {
      std::vector<size_t> next_frontier;
      for (auto u : frontier) {
        for (auto&& [v] : G[u]) {
          if (component[v] == std::numeric_limits<size_t>::max()) {
            component[v] = current_component;
            next_frontier.push_back(v);
          }
        }
      }
      frontier = std::move(next_frontier);
    }

    ++current_component;
  }

  return component;
}

int main() {
  std::cout << "=== Connected Components ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 7" << std::endl << std::endl;

  // Create a graph with multiple connected components
  // Component 0: vertices 0, 1, 2 (triangle)
  // Component 1: vertices 3, 4 (edge)
  // Component 2: vertex 5 (isolated)
  // Component 3: vertices 6, 7, 8 (path)

  std::cout << "Graph structure:" << std::endl;
  std::cout << "  Component 0: 0 - 1 - 2 - 0 (triangle)" << std::endl;
  std::cout << "  Component 1: 3 - 4 (edge)" << std::endl;
  std::cout << "  Component 2: 5 (isolated vertex)" << std::endl;
  std::cout << "  Component 3: 6 - 7 - 8 (path)" << std::endl;
  std::cout << std::endl;

  edge_list<directedness::undirected> edges(9);
  edges.open_for_push_back();
  // Component 0: triangle
  edges.push_back(0, 1);
  edges.push_back(1, 2);
  edges.push_back(2, 0);
  // Component 1: single edge
  edges.push_back(3, 4);
  // Component 2: vertex 5 is isolated (no edges)
  // Component 3: path
  edges.push_back(6, 7);
  edges.push_back(7, 8);
  edges.close_for_push_back();

  adjacency<0> G(edges);

  // Find connected components
  auto component = compute_connected_components_bfs(G);

  // Display results
  std::cout << "Vertex assignments:" << std::endl;
  for (size_t v = 0; v < G.size(); ++v) {
    std::cout << "  Vertex " << v << " -> Component " << component[v] << std::endl;
  }

  // Count components and their sizes
  std::map<size_t, std::vector<size_t>> comp_members;
  for (size_t v = 0; v < G.size(); ++v) {
    comp_members[component[v]].push_back(v);
  }

  std::cout << std::endl;
  std::cout << "Number of connected components: " << comp_members.size() << std::endl;
  std::cout << std::endl;

  std::cout << "Component details:" << std::endl;
  for (auto&& [comp_id, members] : comp_members) {
    std::cout << "  Component " << comp_id << " (size " << members.size() << "): {";
    for (size_t i = 0; i < members.size(); ++i) {
      if (i > 0) std::cout << ", ";
      std::cout << members[i];
    }
    std::cout << "}" << std::endl;
  }

  // Example with a fully connected graph
  std::cout << std::endl;
  std::cout << "=== Fully Connected Graph ===" << std::endl;

  edge_list<directedness::undirected> edges2(5);
  edges2.open_for_push_back();
  for (size_t i = 0; i < 5; ++i) {
    for (size_t j = i + 1; j < 5; ++j) {
      edges2.push_back(i, j);
    }
  }
  edges2.close_for_push_back();

  adjacency<0> G2(edges2);
  auto component2 = compute_connected_components_bfs(G2);

  std::map<size_t, size_t> comp_counts;
  for (auto c : component2) {
    comp_counts[c]++;
  }

  std::cout << "Complete graph K5 has " << comp_counts.size() << " component(s)" << std::endl;
  std::cout << "All " << G2.size() << " vertices belong to component 0" << std::endl;

  return 0;
}
