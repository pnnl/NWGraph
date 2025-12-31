/**
 * @file ch8_maxflow.cpp
 *
 * @brief Maximum Flow (BGL Book Chapter 8)
 *
 * This example demonstrates the maximum flow problem using the
 * Ford-Fulkerson method with BFS (Edmonds-Karp algorithm).
 *
 * Application: Network bandwidth allocation, bipartite matching,
 * transportation networks, supply chain optimization.
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
#include <map>
#include <queue>
#include <vector>

/**
 * @brief BFS to find augmenting path from source to sink
 *
 * @return true if path exists, false otherwise
 */
bool bfs_find_path(const std::vector<std::vector<std::pair<size_t, double>>>& residual,
                   size_t source, size_t sink,
                   std::vector<size_t>& parent) {
  size_t N = residual.size();
  std::vector<bool> visited(N, false);

  std::queue<size_t> Q;
  Q.push(source);
  visited[source] = true;
  parent[source] = source;

  while (!Q.empty()) {
    size_t u = Q.front();
    Q.pop();

    for (auto&& [v, cap] : residual[u]) {
      if (!visited[v] && cap > 0) {
        visited[v] = true;
        parent[v] = u;
        if (v == sink) {
          return true;
        }
        Q.push(v);
      }
    }
  }

  return false;
}

/**
 * @brief Edmonds-Karp algorithm for maximum flow
 *
 * This is the Ford-Fulkerson method using BFS to find augmenting paths.
 * BFS ensures the shortest augmenting path is found, giving O(VE^2) complexity.
 */
double edmonds_karp(size_t N,
                    const std::vector<std::tuple<size_t, size_t, double>>& edges,
                    size_t source, size_t sink) {
  // Build residual graph as adjacency list with capacities
  // We need to track both forward and backward edges
  std::vector<std::vector<std::pair<size_t, double>>> residual(N);

  // Map to find reverse edge quickly
  std::map<std::pair<size_t, size_t>, size_t> edge_index;

  for (auto&& [u, v, cap] : edges) {
    edge_index[{u, v}] = residual[u].size();
    residual[u].push_back({v, cap});

    // Add reverse edge with 0 capacity if it doesn't exist
    if (edge_index.find({v, u}) == edge_index.end()) {
      edge_index[{v, u}] = residual[v].size();
      residual[v].push_back({u, 0});
    }
  }

  std::vector<size_t> parent(N);
  double max_flow = 0;

  // Find augmenting paths until none exist
  while (bfs_find_path(residual, source, sink, parent)) {
    // Find minimum residual capacity along the path
    double path_flow = std::numeric_limits<double>::max();
    for (size_t v = sink; v != source; v = parent[v]) {
      size_t u = parent[v];
      size_t idx = edge_index[{u, v}];
      path_flow = std::min(path_flow, residual[u][idx].second);
    }

    // Update residual capacities
    for (size_t v = sink; v != source; v = parent[v]) {
      size_t u = parent[v];
      size_t fwd_idx = edge_index[{u, v}];
      size_t rev_idx = edge_index[{v, u}];

      residual[u][fwd_idx].second -= path_flow;
      residual[v][rev_idx].second += path_flow;
    }

    max_flow += path_flow;
  }

  return max_flow;
}

int main() {
  std::cout << "=== Maximum Flow Problem ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 8" << std::endl << std::endl;

  // Create a flow network
  // This represents a transportation network where we want to find
  // the maximum flow from source (0) to sink (7)
  //
  //         10        9
  //     0 -----> 1 -----> 5
  //     |        |  \     |
  //   5 |      4 |   \ 15 | 10
  //     v        v    \   v
  //     2 -----> 3 -----> 7
  //     |   4    |   15   ^
  //   8 |      15|       /
  //     v        v      / 10
  //     4 -----> 6 ----/
  //         15

  std::cout << "Flow network (source=0, sink=7):" << std::endl;
  std::cout << "Edges with capacities:" << std::endl;

  std::vector<std::tuple<size_t, size_t, double>> edges = {
      {0, 1, 10},
      {0, 2, 5},
      {0, 3, 15},
      {1, 2, 4},
      {1, 4, 9},
      {1, 5, 15},
      {2, 3, 4},
      {2, 5, 8},
      {3, 6, 30},
      {4, 5, 15},
      {4, 7, 10},
      {5, 6, 15},
      {5, 7, 10},
      {6, 2, 6},
      {6, 5, 4},
      {6, 7, 10}
  };

  for (auto&& [u, v, cap] : edges) {
    std::cout << "  " << u << " -> " << v << " : capacity " << cap << std::endl;
  }
  std::cout << std::endl;

  size_t source = 0;
  size_t sink = 7;
  size_t N = 8;

  double max_flow = edmonds_karp(N, edges, source, sink);

  std::cout << "Maximum flow from " << source << " to " << sink << ": " << max_flow << std::endl;
  std::cout << std::endl;

  // Simpler example
  std::cout << "=== Simple Example ===" << std::endl;
  std::cout << "    10" << std::endl;
  std::cout << "0 -----> 1" << std::endl;
  std::cout << "|        |" << std::endl;
  std::cout << "5|        |10" << std::endl;
  std::cout << "v        v" << std::endl;
  std::cout << "2 -----> 3" << std::endl;
  std::cout << "    10" << std::endl;
  std::cout << std::endl;

  std::vector<std::tuple<size_t, size_t, double>> simple_edges = {
      {0, 1, 10},
      {0, 2, 5},
      {1, 3, 10},
      {2, 3, 10}
  };

  double simple_flow = edmonds_karp(4, simple_edges, 0, 3);
  std::cout << "Maximum flow: " << simple_flow << std::endl;
  std::cout << "(Limited by source outflow: 10 + 5 = 15)" << std::endl;

  return 0;
}
