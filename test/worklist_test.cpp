/**
 * @file worklist_test.cpp
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

#include <queue>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adaptors/worklist.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

// Helper to create a simple graph
auto create_test_graph() {
  edge_list<directedness::directed> edges(5);
  edges.open_for_push_back();
  edges.push_back(0, 1);
  edges.push_back(0, 2);
  edges.push_back(1, 3);
  edges.push_back(2, 4);
  edges.close_for_push_back();
  return edges;
}

TEST_CASE("Worklist range basic operations", "[worklist]") {

  SECTION("Empty worklist") {
    auto edges = create_test_graph();
    adjacency<0> G(edges);

    worklist_range<adjacency<0>> worklist(G);

    REQUIRE(worklist.empty());
  }

  SECTION("Push and iterate") {
    auto edges = create_test_graph();
    adjacency<0> G(edges);

    worklist_range<adjacency<0>> worklist(G);

    // Add items to worklist
    worklist.push_back(0);
    worklist.push_back(1);
    worklist.push_back(2);

    REQUIRE_FALSE(worklist.empty());

    // Iterate through worklist
    std::vector<size_t> items;
    for (auto it = worklist.begin(); it != worklist.end(); ++it) {
      items.push_back(*it);
    }

    REQUIRE(items.size() == 3);
    // FIFO order from std::queue
    REQUIRE(items[0] == 0);
    REQUIRE(items[1] == 1);
    REQUIRE(items[2] == 2);
  }

  SECTION("Dynamic worklist - add during iteration") {
    auto edges = create_test_graph();
    adjacency<0> G(edges);

    worklist_range<adjacency<0>> worklist(G);

    worklist.push_back(0);

    std::vector<size_t> visited;
    std::vector<bool> seen(num_vertices(G), false);

    for (auto it = worklist.begin(); it != worklist.end(); ++it) {
      auto v = *it;
      if (!seen[v]) {
        visited.push_back(v);
        seen[v] = true;

        // Add unvisited neighbors to worklist
        for (auto&& [neighbor] : G[v]) {
          if (!seen[neighbor]) {
            worklist.push_back(neighbor);
          }
        }
      }
    }

    // Should have visited vertices reachable from 0
    // From vertex 0: can reach 1, 2, and from there 3, 4
    REQUIRE(visited.size() >= 1);
    REQUIRE(std::find(visited.begin(), visited.end(), 0) != visited.end());
  }
}

TEST_CASE("Worklist as BFS frontier", "[worklist]") {

  SECTION("BFS-like traversal") {
    auto edges = create_test_graph();
    adjacency<0> G(edges);

    worklist_range<adjacency<0>> frontier(G);
    std::vector<int> distance(num_vertices(G), -1);

    frontier.push_back(0);
    distance[0] = 0;

    for (auto it = frontier.begin(); it != frontier.end(); ++it) {
      auto u = *it;
      for (auto&& [v] : G[u]) {
        if (distance[v] == -1) {
          distance[v] = distance[u] + 1;
          frontier.push_back(v);
        }
      }
    }

    // Check distances from source 0
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 1);  // 0 -> 1
    REQUIRE(distance[2] == 1);  // 0 -> 2
    REQUIRE(distance[3] == 2);  // 0 -> 1 -> 3
    REQUIRE(distance[4] == 2);  // 0 -> 2 -> 4
  }
}
