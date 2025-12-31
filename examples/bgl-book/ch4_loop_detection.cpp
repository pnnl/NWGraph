/**
 * @file ch4_loop_detection.cpp
 *
 * @brief Finding Loops in Program Control-Flow Graphs (BGL Book Chapter 4.2)
 *
 * This example demonstrates using DFS to detect back edges (cycles) in a
 * directed graph. In program analysis, back edges in a control-flow graph
 * indicate loops.
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
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

// Three-way coloring for DFS cycle detection
enum color_t { white_color, grey_color, black_color };

/**
 * @brief Detect if a graph has a cycle using DFS
 *
 * A cycle exists if we find a back edge - an edge from a vertex to one of its
 * ancestors in the DFS tree (a grey vertex).
 *
 * @param G The graph to check
 * @return true if the graph contains a cycle
 */
template <typename Graph>
bool has_cycle_dfs(const Graph& G, size_t u, std::vector<color_t>& color) {
  color[u] = grey_color;

  for (auto&& [v] : G[u]) {
    if (color[v] == grey_color) {
      // Found a back edge - this indicates a cycle
      return true;
    }
    if (color[v] == white_color) {
      if (has_cycle_dfs(G, v, color)) {
        return true;
      }
    }
  }

  color[u] = black_color;
  return false;
}

/**
 * @brief Check if a directed graph contains any cycles
 */
template <typename Graph>
bool has_cycle(const Graph& G) {
  std::vector<color_t> color(G.size(), white_color);

  for (size_t u = 0; u < G.size(); ++u) {
    if (color[u] == white_color) {
      if (has_cycle_dfs(G, u, color)) {
        return true;
      }
    }
  }
  return false;
}

/**
 * @brief Find and print all back edges (loops) in a graph
 */
template <typename Graph>
void find_back_edges(const Graph& G, size_t u, std::vector<color_t>& color,
                     std::vector<std::pair<size_t, size_t>>& back_edges) {
  color[u] = grey_color;

  for (auto&& [v] : G[u]) {
    if (color[v] == grey_color) {
      // This is a back edge
      back_edges.push_back({u, v});
    } else if (color[v] == white_color) {
      find_back_edges(G, v, color, back_edges);
    }
  }

  color[u] = black_color;
}

int main() {
  std::cout << "=== Loop Detection in Control-Flow Graphs ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 4.2" << std::endl << std::endl;

  // Create a simple control-flow graph with a loop
  // This represents a simple while loop:
  //   0 (entry)
  //   |
  //   v
  //   1 (condition) <--+
  //   |                |
  //   v                |
  //   2 (body) --------+
  //   |
  //   v
  //   3 (exit)

  std::cout << "Graph 1: Simple while loop" << std::endl;
  std::cout << "  0 -> 1 -> 2 -> 1 (loop back)" << std::endl;
  std::cout << "            |" << std::endl;
  std::cout << "            v" << std::endl;
  std::cout << "            3" << std::endl;

  edge_list<directedness::directed> E1(4);
  E1.open_for_push_back();
  E1.push_back(0, 1);  // entry -> condition
  E1.push_back(1, 2);  // condition -> body
  E1.push_back(2, 1);  // body -> condition (back edge - loop!)
  E1.push_back(1, 3);  // condition -> exit
  E1.close_for_push_back();

  adjacency<0> G1(E1);

  std::vector<color_t> color1(G1.size(), white_color);
  std::vector<std::pair<size_t, size_t>> back_edges1;

  for (size_t u = 0; u < G1.size(); ++u) {
    if (color1[u] == white_color) {
      find_back_edges(G1, u, color1, back_edges1);
    }
  }

  std::cout << "Has cycle: " << (has_cycle(G1) ? "yes" : "no") << std::endl;
  std::cout << "Back edges found: " << back_edges1.size() << std::endl;
  for (auto&& [u, v] : back_edges1) {
    std::cout << "  " << u << " -> " << v << " (loop)" << std::endl;
  }
  std::cout << std::endl;

  // Create a DAG (no loops)
  std::cout << "Graph 2: DAG (no loops)" << std::endl;
  std::cout << "  0 -> 1 -> 3" << std::endl;
  std::cout << "  |    |" << std::endl;
  std::cout << "  v    v" << std::endl;
  std::cout << "  2 -> 3" << std::endl;

  edge_list<directedness::directed> E2(4);
  E2.open_for_push_back();
  E2.push_back(0, 1);
  E2.push_back(0, 2);
  E2.push_back(1, 3);
  E2.push_back(2, 3);
  E2.close_for_push_back();

  adjacency<0> G2(E2);

  std::cout << "Has cycle: " << (has_cycle(G2) ? "yes" : "no") << std::endl;
  std::cout << std::endl;

  // Create a graph with nested loops
  std::cout << "Graph 3: Nested loops" << std::endl;
  std::cout << "  0 -> 1 -> 2 -> 3 -> 2 (inner loop)" << std::endl;
  std::cout << "       ^         |" << std::endl;
  std::cout << "       +---------+ (outer loop)" << std::endl;

  edge_list<directedness::directed> E3(5);
  E3.open_for_push_back();
  E3.push_back(0, 1);  // entry
  E3.push_back(1, 2);  // outer loop start
  E3.push_back(2, 3);  // inner loop start
  E3.push_back(3, 2);  // inner loop back edge
  E3.push_back(3, 1);  // outer loop back edge
  E3.push_back(2, 4);  // exit
  E3.close_for_push_back();

  adjacency<0> G3(E3);

  std::vector<color_t> color3(G3.size(), white_color);
  std::vector<std::pair<size_t, size_t>> back_edges3;

  for (size_t u = 0; u < G3.size(); ++u) {
    if (color3[u] == white_color) {
      find_back_edges(G3, u, color3, back_edges3);
    }
  }

  std::cout << "Has cycle: " << (has_cycle(G3) ? "yes" : "no") << std::endl;
  std::cout << "Back edges found: " << back_edges3.size() << std::endl;
  for (auto&& [u, v] : back_edges3) {
    std::cout << "  " << u << " -> " << v << " (loop)" << std::endl;
  }

  return 0;
}
