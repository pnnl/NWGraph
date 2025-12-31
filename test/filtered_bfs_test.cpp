/**
 * @file filtered_bfs_test.cpp
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

#include <functional>
#include <set>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adaptors/filtered_bfs_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;
using namespace nw::graph::filtered_bfs;

TEST_CASE("Filtered BFS three colors", "[filtered_bfs]") {
  SECTION("Color values") {
    REQUIRE(three_colors::black == 0);
    REQUIRE(three_colors::white == 1);
    REQUIRE(three_colors::grey == 2);
  }
}

TEST_CASE("Filtered BFS basic search", "[filtered_bfs]") {

  SECTION("Find target in simple graph") {
    // Graph: 0 -> 1 -> 2 -> 3
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // No filter - accept all edges
    auto no_filter = [](auto v, auto iter) { return false; };

    filtered_bfs_edge_range<adjacency<0>, std::queue<size_t>, decltype(no_filter)> bfs(G, 0, 3, no_filter);

    // Iterate until done (target found or unreachable)
    for (auto it = bfs.begin(); it != bfs.end(); ++it) {
      // The loop exits when done() returns true (found or unreachable)
    }

    // Check if target was found after iteration completes
    REQUIRE(bfs.found());
    REQUIRE(bfs.done());
    REQUIRE(!bfs.unreachable);
  }

  SECTION("Target unreachable") {
    // Disconnected graph: 0 -> 1, 2 -> 3
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    auto no_filter = [](auto v, auto iter) { return false; };

    filtered_bfs_edge_range<adjacency<0>, std::queue<size_t>, decltype(no_filter)> bfs(G, 0, 3, no_filter);

    // Consume all
    for (auto it = bfs.begin(); it != bfs.end(); ++it) {
      // iterate
    }

    REQUIRE(bfs.done());
    REQUIRE(!bfs.found());
    REQUIRE(bfs.unreachable);
  }
}

TEST_CASE("Filtered BFS with filter", "[filtered_bfs]") {

  SECTION("Filter blocks some edges") {
    // Graph: 0 -> 1, 0 -> 2, 1 -> 3, 2 -> 3
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(1, 3);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Filter that blocks edges to vertex 1
    auto filter = [](auto v, auto iter) {
      return std::get<0>(*iter) == 1;
    };

    filtered_bfs_edge_range<adjacency<0>, std::queue<size_t>, decltype(filter)> bfs(G, 0, 3, filter);

    std::set<size_t> visited;
    for (auto it = bfs.begin(); it != bfs.end(); ++it) {
      auto neighbor = std::get<0>(*it);
      visited.insert(neighbor);
    }

    // Should NOT visit vertex 1 due to filter
    REQUIRE(visited.count(1) == 0);
    // Should find path through vertex 2
    REQUIRE(visited.count(2) == 1);
  }
}

TEST_CASE("Filtered BFS empty and done states", "[filtered_bfs]") {

  SECTION("Empty check") {
    edge_list<directedness::directed> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    auto no_filter = [](auto v, auto iter) { return false; };

    filtered_bfs_edge_range<adjacency<0>, std::queue<size_t>, decltype(no_filter)> bfs(G, 0, 1, no_filter);

    REQUIRE(!bfs.empty());
  }

  SECTION("Done when target found") {
    edge_list<directedness::directed> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    auto no_filter = [](auto v, auto iter) { return false; };

    filtered_bfs_edge_range<adjacency<0>, std::queue<size_t>, decltype(no_filter)> bfs(G, 0, 1, no_filter);

    for (auto it = bfs.begin(); it != bfs.end(); ++it) {
      // iterate until done
    }

    REQUIRE(bfs.done());
  }
}
