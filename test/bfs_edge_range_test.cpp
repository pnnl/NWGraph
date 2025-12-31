/**
 * @file bfs_edge_range_test.cpp
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

#include <set>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

TEST_CASE("BFS edge range basic", "[bfs_edge_range]") {

  SECTION("Simple tree traversal") {
    // Create a simple tree: 0 -> 1, 0 -> 2, 1 -> 3, 1 -> 4
    edge_list<directedness::directed> edges(5);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(1, 3);
    edges.push_back(1, 4);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    bfs_edge_range<adjacency<0>> bfs(G, 0);

    std::vector<std::pair<size_t, size_t>> visited_edges;
    for (auto it = bfs.begin(); it != bfs.end(); ++it) {
      auto [u, v] = *it;
      visited_edges.push_back({u, v});
    }

    // Should visit all 4 edges
    REQUIRE(visited_edges.size() == 4);

    // First edges should be from vertex 0
    REQUIRE(visited_edges[0].first == 0);
    REQUIRE(visited_edges[1].first == 0);
  }

  SECTION("Linear graph") {
    // Create linear graph: 0 -> 1 -> 2 -> 3
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    bfs_edge_range<adjacency<0>> bfs(G, 0);

    std::vector<std::pair<size_t, size_t>> visited_edges;
    for (auto it = bfs.begin(); it != bfs.end(); ++it) {
      auto [u, v] = *it;
      visited_edges.push_back({u, v});
    }

    REQUIRE(visited_edges.size() == 3);
    REQUIRE(visited_edges[0] == std::make_pair(0ul, 1ul));
    REQUIRE(visited_edges[1] == std::make_pair(1ul, 2ul));
    REQUIRE(visited_edges[2] == std::make_pair(2ul, 3ul));
  }
}

TEST_CASE("BFS edge range with cycles", "[bfs_edge_range]") {

  SECTION("Graph with cycle") {
    // Create graph with cycle: 0 -> 1 -> 2 -> 0
    edge_list<directedness::directed> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.push_back(2, 0);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    bfs_edge_range<adjacency<0>> bfs(G, 0);

    std::set<size_t> visited_vertices;
    for (auto it = bfs.begin(); it != bfs.end(); ++it) {
      auto [u, v] = *it;
      visited_vertices.insert(u);
      visited_vertices.insert(v);
    }

    // Should visit all 3 vertices without infinite loop
    REQUIRE(visited_vertices.size() == 3);
  }
}

TEST_CASE("BFS edge range empty check", "[bfs_edge_range]") {

  SECTION("Empty after traversal") {
    edge_list<directedness::directed> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    bfs_edge_range<adjacency<0>> bfs(G, 0);

    REQUIRE(!bfs.empty());

    // Consume all edges
    for (auto it = bfs.begin(); it != bfs.end(); ++it) {
      // just iterate
    }

    REQUIRE(bfs.empty());
  }
}

TEST_CASE("Three colors enum", "[bfs_edge_range]") {
  REQUIRE(three_colors::black == 0);
  REQUIRE(three_colors::white == 1);
  REQUIRE(three_colors::grey == 2);
}
