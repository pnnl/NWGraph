/**
 * @file ch6_kruskal.cpp
 *
 * @brief Kruskal's Minimum Spanning Tree Algorithm (BGL Book Chapter 6)
 *
 * This example demonstrates Kruskal's algorithm for finding a minimum spanning
 * tree of an undirected weighted graph. The algorithm works by sorting edges
 * by weight and adding them to the tree if they don't create a cycle.
 *
 * Application: Network design - finding the minimum cost way to connect all
 * nodes in a network (e.g., laying cable between cities).
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
#include <numeric>
#include <vector>

#include "nwgraph/algorithms/kruskal.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

int main() {
  std::cout << "=== Kruskal's Minimum Spanning Tree ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 6" << std::endl << std::endl;

  // Create a weighted undirected graph
  // This represents a network where we want to find the minimum cost
  // to connect all nodes.
  //
  //     1---7---2
  //    /|      /|\
  //   5 9     8 7 5
  //  /  |    /  |  \
  // 0   |   /   |   5
  //  \  |  /    |  /
  //   5 15     9  11
  //    \|/      |/
  //     4---6---6
  //
  // Vertex names (for reference):
  // 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G

  std::cout << "Graph: 7 vertices representing cities" << std::endl;
  std::cout << "Edges represent possible cable routes with costs" << std::endl;
  std::cout << std::endl;

  edge_list<directedness::undirected, double> edges(7);
  edges.open_for_push_back();
  edges.push_back(0, 1, 7.0);   // A-B: cost 7
  edges.push_back(0, 3, 5.0);   // A-D: cost 5
  edges.push_back(1, 2, 8.0);   // B-C: cost 8
  edges.push_back(1, 3, 9.0);   // B-D: cost 9
  edges.push_back(1, 4, 7.0);   // B-E: cost 7
  edges.push_back(2, 4, 5.0);   // C-E: cost 5
  edges.push_back(3, 4, 15.0);  // D-E: cost 15
  edges.push_back(3, 5, 6.0);   // D-F: cost 6
  edges.push_back(4, 5, 8.0);   // E-F: cost 8
  edges.push_back(4, 6, 9.0);   // E-G: cost 9
  edges.push_back(5, 6, 11.0);  // F-G: cost 11
  edges.close_for_push_back();

  std::cout << "Input edges (sorted by weight for Kruskal):" << std::endl;
  std::vector<std::tuple<size_t, size_t, double>> sorted_edges;
  for (auto&& [u, v, w] : edges) {
    sorted_edges.push_back({u, v, w});
  }
  std::sort(sorted_edges.begin(), sorted_edges.end(),
            [](auto& a, auto& b) { return std::get<2>(a) < std::get<2>(b); });

  for (auto&& [u, v, w] : sorted_edges) {
    std::cout << "  " << u << " -- " << v << " : " << w << std::endl;
  }
  std::cout << std::endl;

  // Run Kruskal's algorithm
  auto mst = kruskal(edges);

  // Display MST edges
  std::cout << "Minimum Spanning Tree edges:" << std::endl;
  double total_weight = 0.0;
  for (auto&& [u, v, w] : mst) {
    std::cout << "  " << u << " -- " << v << " : " << w << std::endl;
    total_weight += w;
  }

  std::cout << std::endl;
  std::cout << "Total MST weight: " << total_weight << std::endl;
  std::cout << "Number of MST edges: " << std::distance(mst.begin(), mst.end()) << std::endl;
  std::cout << "(A tree with N vertices has N-1 edges)" << std::endl;

  // Also demonstrate maximum spanning tree
  std::cout << std::endl;
  std::cout << "=== Maximum Spanning Tree ===" << std::endl;

  // Recreate edge list (kruskal modifies it)
  edge_list<directedness::undirected, double> edges2(7);
  edges2.open_for_push_back();
  edges2.push_back(0, 1, 7.0);
  edges2.push_back(0, 3, 5.0);
  edges2.push_back(1, 2, 8.0);
  edges2.push_back(1, 3, 9.0);
  edges2.push_back(1, 4, 7.0);
  edges2.push_back(2, 4, 5.0);
  edges2.push_back(3, 4, 15.0);
  edges2.push_back(3, 5, 6.0);
  edges2.push_back(4, 5, 8.0);
  edges2.push_back(4, 6, 9.0);
  edges2.push_back(5, 6, 11.0);
  edges2.close_for_push_back();

  auto max_compare = [](auto t1, auto t2) { return std::get<2>(t1) > std::get<2>(t2); };
  auto max_st = kruskal(edges2, max_compare);

  std::cout << "Maximum Spanning Tree edges:" << std::endl;
  total_weight = 0.0;
  for (auto&& [u, v, w] : max_st) {
    std::cout << "  " << u << " -- " << v << " : " << w << std::endl;
    total_weight += w;
  }
  std::cout << std::endl;
  std::cout << "Total MaxST weight: " << total_weight << std::endl;

  return 0;
}
