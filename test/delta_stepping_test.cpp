/**
 * @file delta_stepping_test.cpp
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
#include "nwgraph/algorithms/delta_stepping.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

// Helper to create a simple weighted graph for testing
// Graph structure:
//     1
//   0 ---> 1
//   |      |
// 4 |      | 2
//   v      v
//   3 <--- 2
//     1
auto create_simple_weighted_graph() {
  edge_list<directedness::directed, double> edges(4);
  edges.open_for_push_back();
  edges.push_back(0, 1, 1.0);  // 0 -> 1, weight 1
  edges.push_back(0, 3, 4.0);  // 0 -> 3, weight 4
  edges.push_back(1, 2, 2.0);  // 1 -> 2, weight 2
  edges.push_back(2, 3, 1.0);  // 2 -> 3, weight 1
  edges.close_for_push_back();
  return edges;
}

// Helper to create a diamond graph with multiple shortest paths
auto create_diamond_graph() {
  //       1 (weight 1)
  //     /   \
  //    0     3
  //     \   /
  //       2 (weight 1)
  edge_list<directedness::directed, double> edges(4);
  edges.open_for_push_back();
  edges.push_back(0, 1, 1.0);
  edges.push_back(0, 2, 1.0);
  edges.push_back(1, 3, 1.0);
  edges.push_back(2, 3, 1.0);
  edges.close_for_push_back();
  return edges;
}

TEST_CASE("Delta-stepping basic functionality", "[delta_stepping]") {

  SECTION("Simple graph shortest paths") {
    auto edges = create_simple_weighted_graph();
    adjacency<0, double> G(edges);

    // Delta = 1.0 for fine-grained buckets
    auto distance = delta_stepping<double>(G, 0, 1.0);

    // Expected distances from vertex 0:
    // 0 -> 0: distance 0
    // 0 -> 1: distance 1 (direct edge)
    // 0 -> 2: distance 3 (0->1->2)
    // 0 -> 3: distance 4 (0->1->2->3, or 0->3 directly)
    REQUIRE(distance[0] == 0.0);
    REQUIRE(distance[1] == 1.0);
    REQUIRE(distance[2] == 3.0);
    REQUIRE(distance[3] == 4.0);
  }

  SECTION("Diamond graph - multiple shortest paths") {
    auto edges = create_diamond_graph();
    adjacency<0, double> G(edges);

    auto distance = delta_stepping<double>(G, 0, 1.0);

    // Both paths 0->1->3 and 0->2->3 have length 2
    REQUIRE(distance[0] == 0.0);
    REQUIRE(distance[1] == 1.0);
    REQUIRE(distance[2] == 1.0);
    REQUIRE(distance[3] == 2.0);
  }

  SECTION("Unreachable vertices") {
    edge_list<directedness::directed, double> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1, 1.0);
    // vertices 2 and 3 are disconnected
    edges.push_back(2, 3, 1.0);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    auto distance = delta_stepping<double>(G, 0, 1.0);

    REQUIRE(distance[0] == 0.0);
    REQUIRE(distance[1] == 1.0);
    // Vertices 2 and 3 should remain unreachable (max value)
    REQUIRE(distance[2] == std::numeric_limits<double>::max());
    REQUIRE(distance[3] == std::numeric_limits<double>::max());
  }
}

TEST_CASE("Delta-stepping with different delta values", "[delta_stepping]") {

  SECTION("Large delta - coarse buckets") {
    auto edges = create_simple_weighted_graph();
    adjacency<0, double> G(edges);

    // Large delta puts many vertices in same bucket
    auto distance = delta_stepping<double>(G, 0, 10.0);

    // Results should be same regardless of delta
    REQUIRE(distance[0] == 0.0);
    REQUIRE(distance[1] == 1.0);
    REQUIRE(distance[2] == 3.0);
    REQUIRE(distance[3] == 4.0);
  }

  SECTION("Small delta - fine buckets") {
    auto edges = create_simple_weighted_graph();
    adjacency<0, double> G(edges);

    // Small delta creates many buckets
    auto distance = delta_stepping<double>(G, 0, 0.5);

    // Results should be same regardless of delta
    REQUIRE(distance[0] == 0.0);
    REQUIRE(distance[1] == 1.0);
    REQUIRE(distance[2] == 3.0);
    REQUIRE(distance[3] == 4.0);
  }
}

TEST_CASE("Delta-stepping single vertex", "[delta_stepping]") {

  SECTION("Single vertex graph") {
    edge_list<directedness::directed, double> edges(1);
    adjacency<0, double> G(edges);

    auto distance = delta_stepping<double>(G, 0, 1.0);

    REQUIRE(distance.size() == 1);
    REQUIRE(distance[0] == 0.0);
  }
}

TEST_CASE("Delta-stepping linear chain", "[delta_stepping]") {

  SECTION("Linear chain graph") {
    // 0 -> 1 -> 2 -> 3 -> 4
    edge_list<directedness::directed, double> edges(5);
    edges.open_for_push_back();
    for (size_t i = 0; i < 4; ++i) {
      edges.push_back(i, i + 1, 1.0);
    }
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    auto distance = delta_stepping<double>(G, 0, 1.0);

    for (size_t i = 0; i < 5; ++i) {
      REQUIRE(distance[i] == static_cast<double>(i));
    }
  }
}

TEST_CASE("Delta-stepping varying weights", "[delta_stepping]") {

  SECTION("Zero weight edges") {
    edge_list<directedness::directed, double> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1, 0.0);
    edges.push_back(1, 2, 0.0);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    auto distance = delta_stepping<double>(G, 0, 1.0);

    REQUIRE(distance[0] == 0.0);
    REQUIRE(distance[1] == 0.0);
    REQUIRE(distance[2] == 0.0);
  }

  SECTION("Fractional weights") {
    edge_list<directedness::directed, double> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1, 0.5);
    edges.push_back(1, 2, 0.25);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    auto distance = delta_stepping<double>(G, 0, 0.1);

    REQUIRE(distance[0] == 0.0);
    REQUIRE(distance[1] == Approx(0.5));
    REQUIRE(distance[2] == Approx(0.75));
  }
}

TEST_CASE("Delta-stepping non-zero source", "[delta_stepping]") {

  SECTION("Starting from middle vertex") {
    auto edges = create_simple_weighted_graph();
    adjacency<0, double> G(edges);

    // Start from vertex 1
    auto distance = delta_stepping<double>(G, 1, 1.0);

    // vertex 0 is unreachable from 1
    REQUIRE(distance[0] == std::numeric_limits<double>::max());
    REQUIRE(distance[1] == 0.0);  // source
    REQUIRE(distance[2] == 2.0);  // 1->2
    REQUIRE(distance[3] == 3.0);  // 1->2->3
  }
}
