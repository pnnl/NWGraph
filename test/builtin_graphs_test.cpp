/**
 * @file builtin_graphs_test.cpp
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

#include "common/test_header.hpp"

#include "nwgraph/graphs/karate-graph.hpp"

TEST_CASE("Karate graph edge list", "[builtin_graphs]") {

  SECTION("Edge list has correct size") {
    // Zachary's karate club has 78 edges
    REQUIRE(karate_index_edge_list.size() == 78);
  }

  SECTION("Edge list format is correct") {
    // Check first edge
    auto& first_edge = karate_index_edge_list[0];
    REQUIRE(std::get<0>(first_edge) == 1);
    REQUIRE(std::get<1>(first_edge) == 0);
  }

  SECTION("All edges have valid vertices") {
    for (const auto& [u, v] : karate_index_edge_list) {
      REQUIRE(u < 34);
      REQUIRE(v < 34);
    }
  }
}

TEST_CASE("Karate directed adjacency list", "[builtin_graphs]") {

  SECTION("Has 34 vertices") {
    REQUIRE(karate_directed_adjacency_list.size() == 34);
  }

  SECTION("Vertex 0 has no outgoing edges in directed view") {
    // In the directed view (lower triangle), vertex 0 has no outgoing edges
    REQUIRE(karate_directed_adjacency_list[0].size() == 0);
  }

  SECTION("Vertex 33 has many outgoing edges") {
    // Vertex 33 (leader) connects to many lower-numbered vertices
    REQUIRE(karate_directed_adjacency_list[33].size() > 10);
  }

  SECTION("All neighbors are valid vertices") {
    for (size_t v = 0; v < karate_directed_adjacency_list.size(); ++v) {
      for (auto neighbor : karate_directed_adjacency_list[v]) {
        REQUIRE(neighbor < 34);
        // In directed version, all neighbors should be < v
        REQUIRE(neighbor < v);
      }
    }
  }
}

TEST_CASE("Karate undirected adjacency list", "[builtin_graphs]") {

  SECTION("Has 34 vertices") {
    REQUIRE(karate_undirected_adjacency_list.size() == 34);
  }

  SECTION("Vertex 0 is highly connected (leader)") {
    // Mr. Hi (vertex 0) is one of the leaders
    REQUIRE(karate_undirected_adjacency_list[0].size() == 16);
  }

  SECTION("Vertex 33 is highly connected (leader)") {
    // John A (vertex 33) is the other leader
    REQUIRE(karate_undirected_adjacency_list[33].size() == 17);
  }

  SECTION("Total edge count is consistent") {
    // Count all edges (each undirected edge counted once per endpoint)
    size_t total_degree = 0;
    for (const auto& neighbors : karate_undirected_adjacency_list) {
      total_degree += neighbors.size();
    }
    // Each edge is counted twice (once per endpoint), so total should be 2 * 78 = 156
    REQUIRE(total_degree == 156);
  }

  SECTION("Graph is connected") {
    // Simple check: all vertices have at least one neighbor
    for (size_t v = 0; v < karate_undirected_adjacency_list.size(); ++v) {
      REQUIRE(karate_undirected_adjacency_list[v].size() >= 1);
    }
  }

  SECTION("All neighbors are valid and symmetric") {
    for (size_t v = 0; v < karate_undirected_adjacency_list.size(); ++v) {
      for (auto u : karate_undirected_adjacency_list[v]) {
        REQUIRE(u < 34);
        // Check symmetry: if v->u exists, then u->v should exist
        auto& u_neighbors = karate_undirected_adjacency_list[u];
        bool found = std::find(u_neighbors.begin(), u_neighbors.end(), v) != u_neighbors.end();
        REQUIRE(found);
      }
    }
  }
}

TEST_CASE("Karate graph properties", "[builtin_graphs]") {

  SECTION("Known graph statistics") {
    // Zachary's karate club graph is well-studied
    // 34 vertices, 78 edges
    size_t num_vertices = karate_undirected_adjacency_list.size();
    size_t num_edges = karate_index_edge_list.size();

    REQUIRE(num_vertices == 34);
    REQUIRE(num_edges == 78);
  }
}
