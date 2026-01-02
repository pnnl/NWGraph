/**
 * @file betweenness_centrality_test.cpp
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

#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adjacency.hpp"
#include "nwgraph/algorithms/betweenness_centrality.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

// Helper to create a simple line graph: 0 -- 1 -- 2 -- 3 -- 4
// In a line graph, the middle vertex has highest betweenness centrality
auto create_line_graph() {
  edge_list<directedness::directed> edges(5);
  edges.open_for_push_back();
  // Bidirectional edges for undirected behavior
  edges.push_back(0, 1);
  edges.push_back(1, 0);
  edges.push_back(1, 2);
  edges.push_back(2, 1);
  edges.push_back(2, 3);
  edges.push_back(3, 2);
  edges.push_back(3, 4);
  edges.push_back(4, 3);
  edges.close_for_push_back();
  return edges;
}

// Helper to create a star graph with center at vertex 0
// Center has highest BC, all leaves have zero BC
auto create_star_graph() {
  edge_list<directedness::directed> edges(5);
  edges.open_for_push_back();
  // 0 is the center, connected to 1, 2, 3, 4
  edges.push_back(0, 1);
  edges.push_back(1, 0);
  edges.push_back(0, 2);
  edges.push_back(2, 0);
  edges.push_back(0, 3);
  edges.push_back(3, 0);
  edges.push_back(0, 4);
  edges.push_back(4, 0);
  edges.close_for_push_back();
  return edges;
}

// Helper to create a simple triangle graph
auto create_triangle_graph() {
  edge_list<directedness::directed> edges(3);
  edges.open_for_push_back();
  edges.push_back(0, 1);
  edges.push_back(1, 0);
  edges.push_back(1, 2);
  edges.push_back(2, 1);
  edges.push_back(2, 0);
  edges.push_back(0, 2);
  edges.close_for_push_back();
  return edges;
}

TEST_CASE("Brandes betweenness centrality basic", "[betweenness]") {

  SECTION("Line graph - middle vertex has highest BC") {
    auto edges = create_line_graph();
    adjacency<0> G(edges);

    auto bc = brandes_bc<adjacency<0>, float, size_t>(G, false);

    REQUIRE(bc.size() == 5);

    // In a line graph 0-1-2-3-4, vertex 2 is on the most shortest paths
    // Vertex 0 and 4 have BC = 0 (endpoints)
    // Vertex 2 should have highest BC
    REQUIRE(bc[0] == 0.0f);
    REQUIRE(bc[4] == 0.0f);
    REQUIRE(bc[2] > bc[1]);
    REQUIRE(bc[2] > bc[3]);
    // Symmetry: bc[1] == bc[3]
    REQUIRE(bc[1] == bc[3]);
  }

  SECTION("Star graph - center has highest BC") {
    auto edges = create_star_graph();
    adjacency<0> G(edges);

    auto bc = brandes_bc<adjacency<0>, float, size_t>(G, false);

    REQUIRE(bc.size() == 5);

    // All shortest paths between leaf nodes go through center (vertex 0)
    // Leaf nodes have BC = 0
    REQUIRE(bc[1] == 0.0f);
    REQUIRE(bc[2] == 0.0f);
    REQUIRE(bc[3] == 0.0f);
    REQUIRE(bc[4] == 0.0f);
    // Center should have highest BC
    REQUIRE(bc[0] > 0.0f);
  }

  SECTION("Triangle graph - all vertices have equal BC") {
    auto edges = create_triangle_graph();
    adjacency<0> G(edges);

    auto bc = brandes_bc<adjacency<0>, float, size_t>(G, false);

    REQUIRE(bc.size() == 3);

    // In a complete triangle, all vertices have same BC (0)
    // because there are direct edges between all pairs
    REQUIRE(bc[0] == 0.0f);
    REQUIRE(bc[1] == 0.0f);
    REQUIRE(bc[2] == 0.0f);
  }
}

TEST_CASE("Brandes BC with normalization", "[betweenness]") {

  SECTION("Normalized BC values are in [0,1]") {
    auto edges = create_line_graph();
    adjacency<0> G(edges);

    auto bc = brandes_bc<adjacency<0>, float, size_t>(G, true);

    REQUIRE(bc.size() == 5);

    for (auto& score : bc) {
      REQUIRE(score >= 0.0f);
      REQUIRE(score <= 1.0f);
    }

    // Highest BC vertex should have score 1.0 after normalization
    float max_bc = *std::max_element(bc.begin(), bc.end());
    REQUIRE(max_bc == 1.0f);
  }
}

TEST_CASE("BC single vertex graph", "[betweenness]") {

  SECTION("Single vertex has BC = 0") {
    edge_list<directedness::directed> edges(1);
    adjacency<0> G(edges);

    auto bc = brandes_bc<adjacency<0>, float, size_t>(G, false);

    REQUIRE(bc.size() == 1);
    REQUIRE(bc[0] == 0.0f);
  }
}

TEST_CASE("BC disconnected graph", "[betweenness]") {

  SECTION("Disconnected components") {
    // Two separate edges: 0-1 and 2-3
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 0);
    edges.push_back(2, 3);
    edges.push_back(3, 2);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    auto bc = brandes_bc<adjacency<0>, float, size_t>(G, false);

    REQUIRE(bc.size() == 4);

    // All vertices have BC = 0 (no vertex lies on paths between others)
    REQUIRE(bc[0] == 0.0f);
    REQUIRE(bc[1] == 0.0f);
    REQUIRE(bc[2] == 0.0f);
    REQUIRE(bc[3] == 0.0f);
  }
}
