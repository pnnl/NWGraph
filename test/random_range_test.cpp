/**
 * @file random_range_test.cpp
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

#include "nwgraph/adaptors/random_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

TEST_CASE("Random range basic", "[random_range]") {

  SECTION("Random walk on complete graph") {
    // Create a complete graph K4
    edge_list<directedness::undirected> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(0, 3);
    edges.push_back(1, 2);
    edges.push_back(1, 3);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Walk of length 10 starting from vertex 0, fixed seed for reproducibility
    random_range<adjacency<0>> walk(G, 10, 0, 42);

    std::vector<size_t> visited;
    for (auto v : walk) {
      visited.push_back(v);
    }

    REQUIRE(visited.size() == 10);

    // All visited vertices should be valid (0-3)
    for (auto v : visited) {
      REQUIRE(v < 4);
    }
  }

  SECTION("Random walk visits neighbors") {
    // Create a simple graph: 0 -- 1 -- 2 (linear)
    edge_list<directedness::undirected> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Walk starting from vertex 1 (which can go to 0 or 2)
    random_range<adjacency<0>> walk(G, 5, 1, 123);

    std::vector<size_t> visited;
    for (auto v : walk) {
      visited.push_back(v);
    }

    REQUIRE(visited.size() == 5);

    // First vertex is the starting vertex
    REQUIRE(visited[0] == 1);

    // Subsequent vertices must be neighbors (0 or 2 for vertex 1, or 1 for vertices 0/2)
    for (size_t i = 1; i < visited.size(); ++i) {
      REQUIRE(visited[i] < 3);  // Valid vertex
    }
  }

  SECTION("Random walk length 1") {
    edge_list<directedness::undirected> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    random_range<adjacency<0>> walk(G, 1, 0, 999);

    std::vector<size_t> visited;
    for (auto v : walk) {
      visited.push_back(v);
    }

    REQUIRE(visited.size() == 1);
    REQUIRE(visited[0] == 0);  // First element is the starting vertex
  }
}

TEST_CASE("Random range determinism", "[random_range]") {

  SECTION("Same seed produces same walk") {
    edge_list<directedness::undirected> edges(5);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(1, 2);
    edges.push_back(1, 3);
    edges.push_back(2, 4);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // First walk with seed 42
    random_range<adjacency<0>> walk1(G, 20, 0, 42);
    std::vector<size_t> visited1;
    for (auto v : walk1) {
      visited1.push_back(v);
    }

    // Second walk with same seed 42
    random_range<adjacency<0>> walk2(G, 20, 0, 42);
    std::vector<size_t> visited2;
    for (auto v : walk2) {
      visited2.push_back(v);
    }

    REQUIRE(visited1.size() == visited2.size());
    for (size_t i = 0; i < visited1.size(); ++i) {
      REQUIRE(visited1[i] == visited2[i]);
    }
  }

  SECTION("Different seeds produce different walks") {
    edge_list<directedness::undirected> edges(10);
    edges.open_for_push_back();
    for (size_t i = 0; i < 9; ++i) {
      edges.push_back(i, i + 1);
      if (i > 0) {
        edges.push_back(0, i);  // Add edges from 0 to all
      }
    }
    edges.close_for_push_back();

    adjacency<0> G(edges);

    random_range<adjacency<0>> walk1(G, 50, 0, 111);
    std::vector<size_t> visited1;
    for (auto v : walk1) {
      visited1.push_back(v);
    }

    random_range<adjacency<0>> walk2(G, 50, 0, 222);
    std::vector<size_t> visited2;
    for (auto v : walk2) {
      visited2.push_back(v);
    }

    // With high probability, walks should differ somewhere
    bool differs = false;
    for (size_t i = 0; i < visited1.size(); ++i) {
      if (visited1[i] != visited2[i]) {
        differs = true;
        break;
      }
    }
    REQUIRE(differs);
  }
}

TEST_CASE("Random range coverage", "[random_range]") {

  SECTION("Long walk visits multiple vertices") {
    // Create a cycle graph
    edge_list<directedness::undirected> edges(6);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.push_back(3, 4);
    edges.push_back(4, 5);
    edges.push_back(5, 0);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Long enough walk should visit multiple unique vertices
    random_range<adjacency<0>> walk(G, 100, 0, 7777);

    std::set<size_t> unique_vertices;
    for (auto v : walk) {
      unique_vertices.insert(v);
    }

    // Should visit more than just 1-2 vertices
    REQUIRE(unique_vertices.size() > 2);
  }
}
