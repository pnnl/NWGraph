/**
 * @file range_adaptors_test.cpp
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

#include <algorithm>
#include <set>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/neighbor_range.hpp"
#include "nwgraph/adaptors/plain_range.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

// Helper to create a simple directed graph
//   0 --> 1 --> 2
//   |           ^
//   +--> 3 -----+
auto create_simple_graph() {
  edge_list<directedness::directed> edges(4);
  edges.open_for_push_back();
  edges.push_back(0, 1);
  edges.push_back(0, 3);
  edges.push_back(1, 2);
  edges.push_back(3, 2);
  edges.close_for_push_back();
  return edges;
}

// Helper to create a weighted graph
auto create_weighted_graph() {
  edge_list<directedness::directed, double> edges(4);
  edges.open_for_push_back();
  edges.push_back(0, 1, 1.0);
  edges.push_back(0, 2, 2.0);
  edges.push_back(1, 2, 3.0);
  edges.push_back(2, 3, 4.0);
  edges.close_for_push_back();
  return edges;
}

// =============================================================================
// Plain Range Tests
// =============================================================================

TEST_CASE("Plain range basic iteration", "[plain_range]") {

  SECTION("Iterate over all vertices") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    plain_range<adjacency<0>> range(G);

    std::vector<size_t> vertices;
    for (auto&& [v] : range) {
      vertices.push_back(v);
    }

    // Verify we visit all actual vertices (0, 1, 2, 3)
    size_t n = num_vertices(G);
    for (size_t i = 0; i < n; ++i) {
      REQUIRE(std::find(vertices.begin(), vertices.end(), i) != vertices.end());
    }
  }

  SECTION("Plain range size and empty") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    plain_range<adjacency<0>> range(G);

    // Range should not be empty
    REQUIRE_FALSE(range.empty());
    // Size should be positive
    REQUIRE(range.size() > 0);
  }

  SECTION("make_plain_range helper") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    auto range = make_plain_range(G);

    size_t count = 0;
    for (auto&& [v] : range) {
      (void)v;
      count++;
    }
    REQUIRE(count == range.size());
  }
}

TEST_CASE("Plain degree range", "[plain_range]") {

  SECTION("Iterate with degrees") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    plain_degree_range range(G);

    std::map<size_t, size_t> vertex_degrees;
    for (auto&& [v, deg] : range) {
      vertex_degrees[v] = deg;
    }

    // Check that expected vertices have correct degrees
    REQUIRE(vertex_degrees[0] == 2);  // 0 -> 1, 3
    REQUIRE(vertex_degrees[1] == 1);  // 1 -> 2
    REQUIRE(vertex_degrees[2] == 0);  // 2 has no outgoing
    REQUIRE(vertex_degrees[3] == 1);  // 3 -> 2
  }
}

// =============================================================================
// Edge Range Tests
// =============================================================================

TEST_CASE("Edge range basic iteration", "[edge_range]") {

  SECTION("Iterate over all edges") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    edge_range<adjacency<0>> range(G);

    std::vector<std::pair<size_t, size_t>> edge_list;
    for (auto it = range.begin(); it != range.end(); ++it) {
      auto [u, v] = *it;
      edge_list.push_back({u, v});
    }

    // We have 4 edges in our graph
    REQUIRE(edge_list.size() == 4);
  }

  SECTION("Edge range with weighted graph") {
    auto edges = create_weighted_graph();
    adjacency<0, double> G(edges);

    edge_range<adjacency<0, double>, 0> range(G);

    double weight_sum = 0.0;
    size_t edge_count = 0;
    for (auto it = range.begin(); it != range.end(); ++it) {
      auto [u, v, w] = *it;
      (void)u;
      (void)v;
      weight_sum += w;
      edge_count++;
    }

    REQUIRE(edge_count == 4);
    REQUIRE(weight_sum == Approx(10.0));  // 1 + 2 + 3 + 4
  }

  SECTION("make_edge_range helper") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    auto range = make_edge_range(G);

    size_t count = 0;
    for (auto it = range.begin(); it != range.end(); ++it) {
      count++;
    }
    REQUIRE(count == 4);
  }

  SECTION("Edge range not empty") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    edge_range<adjacency<0>> range(G);

    REQUIRE_FALSE(range.empty());
    REQUIRE(range.size() > 0);
  }
}

// =============================================================================
// Neighbor Range Tests
// =============================================================================

TEST_CASE("Neighbor range basic iteration", "[neighbor_range]") {

  SECTION("Iterate with vertex and neighbors") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    neighbor_range<adjacency<0>> range(G);

    size_t vertex_count = 0;
    for (auto it = range.begin(); it != range.end(); ++it) {
      auto [v, neighbors] = *it;
      (void)v;
      (void)neighbors;
      vertex_count++;
    }

    REQUIRE(vertex_count == range.size());
  }

  SECTION("Neighbor range with offset") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    neighbor_range<adjacency<0>> full_range(G);
    size_t offset = 1;
    neighbor_range<adjacency<0>> range(G, offset);

    size_t count = 0;
    for (auto it = range.begin(); it != range.end(); ++it) {
      count++;
    }
    REQUIRE(count == full_range.size() - offset);
  }

  SECTION("make_neighbor_range helper") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    auto range = make_neighbor_range(G);

    REQUIRE_FALSE(range.empty());
    REQUIRE(range.size() > 0);
  }
}

// =============================================================================
// Iterator Tests
// =============================================================================

TEST_CASE("Range iterator operations", "[range_iterators]") {

  SECTION("Plain range iterator comparison") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    plain_range<adjacency<0>> range(G);

    auto it1 = range.begin();
    auto it2 = range.begin();
    auto end = range.end();

    REQUIRE(it1 == it2);
    REQUIRE(it1 != end);

    ++it1;
    REQUIRE(it1 != it2);
  }

  SECTION("Edge range iterator comparison") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    edge_range<adjacency<0>> range(G);

    auto it1 = range.begin();
    auto it2 = range.begin();
    auto end = range.end();

    REQUIRE(it1 == it2);
    REQUIRE_FALSE(it1 == end);
  }

  SECTION("Range iterator arithmetic") {
    auto edges = create_simple_graph();
    adjacency<0> G(edges);

    plain_range<adjacency<0>> range(G);

    auto it1 = range.begin();
    auto it2 = range.begin() + 2;

    REQUIRE(it2 - it1 == 2);
  }
}

// =============================================================================
// Empty Graph Tests
// =============================================================================

TEST_CASE("Range adaptors on empty graph", "[range_empty]") {

  SECTION("Plain range on empty graph") {
    edge_list<directedness::directed> edges(0);
    adjacency<0> G(edges);

    plain_range<adjacency<0>> range(G);

    // Empty graph iteration should yield no actual vertex data
    size_t count = 0;
    for (auto it = range.begin(); it != range.end(); ++it) {
      count++;
    }
    // With 0 vertices, iteration should be minimal
    REQUIRE(count <= 1);  // May have a sentinel element
  }

  SECTION("Edge range on empty graph") {
    edge_list<directedness::directed> edges(0);
    adjacency<0> G(edges);

    edge_range<adjacency<0>> range(G);

    // Empty graph should have empty edge range
    size_t edge_count = 0;
    for (auto it = range.begin(); it != range.end(); ++it) {
      edge_count++;
    }
    REQUIRE(edge_count == 0);
  }

  SECTION("Single vertex no edges") {
    edge_list<directedness::directed> edges(1);
    adjacency<0> G(edges);

    edge_range<adjacency<0>> er(G);
    // Should have no edges to iterate
    size_t edge_count = 0;
    for (auto it = er.begin(); it != er.end(); ++it) {
      edge_count++;
    }
    REQUIRE(edge_count == 0);
  }
}

// =============================================================================
// Const Graph Tests
// =============================================================================

TEST_CASE("Range adaptors with const graph", "[range_const]") {

  SECTION("Plain range on const graph") {
    auto edges = create_simple_graph();
    const adjacency<0> G(edges);

    plain_range<const adjacency<0>> range(G);

    size_t count = 0;
    for (auto&& [v] : range) {
      (void)v;
      count++;
    }
    REQUIRE(count == range.size());
  }

  SECTION("Edge range on const graph") {
    auto edges = create_simple_graph();
    const adjacency<0> G(edges);

    edge_range<const adjacency<0>> range(G);

    size_t count = 0;
    for (auto it = range.begin(); it != range.end(); ++it) {
      count++;
    }
    // Edge count should match the actual edges
    REQUIRE(count == 4);
  }
}
