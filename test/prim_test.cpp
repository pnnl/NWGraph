/**
 * @file prim_test.cpp
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

#include <limits>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adjacency.hpp"
#include "nwgraph/algorithms/prim.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

// Helper to create a simple weighted undirected graph for MST testing
// Graph structure (undirected):
//     1
//   0 --- 1
//   |     |
// 4 |     | 2
//   |     |
//   3 --- 2
//     3
// MST should pick edges: 0-1 (1), 1-2 (2), 2-3 (3) = total weight 6
// Not: 0-3 (4) because 0-1-2-3 is cheaper
auto create_simple_weighted_graph() {
  edge_list<directedness::directed, double> edges(4);
  edges.open_for_push_back();
  // Bidirectional edges for undirected behavior
  edges.push_back(0, 1, 1.0);
  edges.push_back(1, 0, 1.0);
  edges.push_back(0, 3, 4.0);
  edges.push_back(3, 0, 4.0);
  edges.push_back(1, 2, 2.0);
  edges.push_back(2, 1, 2.0);
  edges.push_back(2, 3, 3.0);
  edges.push_back(3, 2, 3.0);
  edges.close_for_push_back();
  return edges;
}

// Helper to create a triangle graph
auto create_triangle_graph() {
  edge_list<directedness::directed, double> edges(3);
  edges.open_for_push_back();
  // Triangle with weights: 0-1: 1, 1-2: 2, 0-2: 3
  edges.push_back(0, 1, 1.0);
  edges.push_back(1, 0, 1.0);
  edges.push_back(1, 2, 2.0);
  edges.push_back(2, 1, 2.0);
  edges.push_back(0, 2, 3.0);
  edges.push_back(2, 0, 3.0);
  edges.close_for_push_back();
  return edges;
}

TEST_CASE("Prim MST basic functionality", "[prim]") {

  SECTION("Simple graph MST") {
    auto edges = create_simple_weighted_graph();
    adjacency<0, double> G(edges);

    auto weight = [](auto& e) { return std::get<1>(e); };
    auto predecessor = prim<adjacency<0, double>, double>(G, 0, weight);

    REQUIRE(predecessor.size() == 4);

    // Source has no predecessor (or max value)
    // Check that a spanning tree was formed (n-1 edges for n vertices)
    using vertex_id_type = typename decltype(G)::vertex_id_type;
    size_t edge_count = 0;
    for (size_t i = 0; i < predecessor.size(); ++i) {
      if (predecessor[i] != std::numeric_limits<vertex_id_type>::max()) {
        ++edge_count;
      }
    }
    // Should have n-1 = 3 edges in MST (source doesn't count)
    REQUIRE(edge_count == 3);
  }

  SECTION("Triangle graph MST") {
    auto edges = create_triangle_graph();
    adjacency<0, double> G(edges);

    auto weight = [](auto& e) { return std::get<1>(e); };
    auto predecessor = prim<adjacency<0, double>, double>(G, 0, weight);

    REQUIRE(predecessor.size() == 3);

    // MST of triangle picks two cheapest edges: 0-1 (1) and 1-2 (2)
    // So predecessor[1] = 0, predecessor[2] = 1
    // Total MST weight = 1 + 2 = 3 (not using 0-2 edge with weight 3)
  }
}

TEST_CASE("Prim MST single vertex", "[prim]") {

  SECTION("Single vertex graph") {
    edge_list<directedness::directed, double> edges(1);
    adjacency<0, double> G(edges);

    auto weight = [](auto& e) { return std::get<1>(e); };
    auto predecessor = prim<adjacency<0, double>, double>(G, 0, weight);

    REQUIRE(predecessor.size() == 1);
  }
}

TEST_CASE("Prim MST two vertices", "[prim]") {

  SECTION("Two connected vertices") {
    edge_list<directedness::directed, double> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1, 5.0);
    edges.push_back(1, 0, 5.0);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    auto weight = [](auto& e) { return std::get<1>(e); };
    auto predecessor = prim<adjacency<0, double>, double>(G, 0, weight);

    REQUIRE(predecessor.size() == 2);
    // Vertex 1's predecessor should be 0
    REQUIRE(predecessor[1] == 0);
  }
}

TEST_CASE("Prim MST linear chain", "[prim]") {

  SECTION("Linear chain graph") {
    // 0 -- 1 -- 2 -- 3
    edge_list<directedness::directed, double> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1, 1.0);
    edges.push_back(1, 0, 1.0);
    edges.push_back(1, 2, 1.0);
    edges.push_back(2, 1, 1.0);
    edges.push_back(2, 3, 1.0);
    edges.push_back(3, 2, 1.0);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    auto weight = [](auto& e) { return std::get<1>(e); };
    auto predecessor = prim<adjacency<0, double>, double>(G, 0, weight);

    REQUIRE(predecessor.size() == 4);

    // MST is just the chain itself
    // predecessor[1] = 0, predecessor[2] = 1, predecessor[3] = 2
    REQUIRE(predecessor[1] == 0);
    REQUIRE(predecessor[2] == 1);
    REQUIRE(predecessor[3] == 2);
  }
}

TEST_CASE("Prim MST different source", "[prim]") {

  SECTION("Starting from non-zero source") {
    auto edges = create_simple_weighted_graph();
    adjacency<0, double> G(edges);

    auto weight = [](auto& e) { return std::get<1>(e); };
    // Start from vertex 2
    auto predecessor = prim<adjacency<0, double>, double>(G, 2, weight);

    REQUIRE(predecessor.size() == 4);

    // Count edges in MST
    using vertex_id_type = typename decltype(G)::vertex_id_type;
    size_t edge_count = 0;
    for (size_t i = 0; i < predecessor.size(); ++i) {
      if (predecessor[i] != std::numeric_limits<vertex_id_type>::max()) {
        ++edge_count;
      }
    }
    REQUIRE(edge_count == 3);
  }
}
