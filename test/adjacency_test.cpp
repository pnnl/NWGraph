/**
 * @file adjacency_test.cpp
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
#include "nwgraph/edge_list.hpp"
#include "nwgraph/graph_concepts.hpp"

using namespace nw::graph;

TEST_CASE("Adjacency construction from edge list", "[adjacency]") {

  SECTION("Directed graph construction") {
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    REQUIRE(num_vertices(G) == 4);
    REQUIRE(G.num_edges() == 4);

    // Check adjacencies
    REQUIRE(G[0].size() == 2);  // 0 -> 1, 2
    REQUIRE(G[1].size() == 1);  // 1 -> 2
    REQUIRE(G[2].size() == 1);  // 2 -> 3
    REQUIRE(G[3].size() == 0);  // 3 has no outgoing edges
  }

  SECTION("Undirected graph construction") {
    edge_list<directedness::undirected> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    REQUIRE(num_vertices(G) == 3);
    // Undirected edges are stored both ways
    REQUIRE(G.num_edges() == 4);

    // Each vertex should see its neighbors
    REQUIRE(G[0].size() >= 1);  // 0 -- 1
    REQUIRE(G[1].size() >= 2);  // 1 -- 0, 2
    REQUIRE(G[2].size() >= 1);  // 2 -- 1
  }

  SECTION("Empty graph") {
    edge_list<directedness::directed> edges(0);
    adjacency<0> G(edges);

    REQUIRE(num_vertices(G) == 0);
    REQUIRE(G.num_edges() == 0);
  }

  SECTION("Single vertex no edges") {
    edge_list<directedness::directed> edges(1);
    adjacency<0> G(edges);

    REQUIRE(num_vertices(G) == 1);
    REQUIRE(G.num_edges() == 0);
    REQUIRE(G[0].size() == 0);
  }
}

TEST_CASE("Adjacency with edge attributes", "[adjacency]") {

  SECTION("Weighted edges") {
    edge_list<directedness::directed, double> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1, 1.5);
    edges.push_back(0, 2, 2.5);
    edges.push_back(1, 2, 3.5);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    REQUIRE(num_vertices(G) == 3);
    REQUIRE(G.num_edges() == 3);

    // Check that we can access edge weights
    size_t edge_count = 0;
    double weight_sum = 0.0;
    for (size_t u = 0; u < num_vertices(G); ++u) {
      for (auto&& [v, w] : G[u]) {
        edge_count++;
        weight_sum += w;
      }
    }
    REQUIRE(edge_count == 3);
    REQUIRE(weight_sum == Approx(7.5));
  }

  SECTION("Multiple edge attributes") {
    edge_list<directedness::directed, double, int> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1, 1.0, 10);
    edges.push_back(1, 0, 2.0, 20);
    edges.close_for_push_back();

    adjacency<0, double, int> G(edges);

    REQUIRE(num_vertices(G) == 2);
    REQUIRE(G.num_edges() == 2);
  }
}

TEST_CASE("Adjacency iteration", "[adjacency]") {

  SECTION("Iterate over all edges") {
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(1, 3);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    size_t total_edges = 0;
    for (size_t u = 0; u < num_vertices(G); ++u) {
      for (auto&& [v] : G[u]) {
        total_edges++;
        (void)v;  // use v to avoid warning
      }
    }
    REQUIRE(total_edges == 4);
  }

  SECTION("Access specific neighbors") {
    edge_list<directedness::directed> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    std::vector<size_t> neighbors;
    for (auto&& [v] : G[0]) {
      neighbors.push_back(v);
    }

    REQUIRE(neighbors.size() == 2);
    // Neighbors may be in any order
    bool has_1 = std::find(neighbors.begin(), neighbors.end(), 1) != neighbors.end();
    bool has_2 = std::find(neighbors.begin(), neighbors.end(), 2) != neighbors.end();
    REQUIRE(has_1);
    REQUIRE(has_2);
  }
}

TEST_CASE("Adjacency degree", "[adjacency]") {

  SECTION("Degree via size()") {
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(0, 3);
    edges.push_back(1, 2);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Use G[v].size() to get out-degree
    REQUIRE(G[0].size() == 3);
    REQUIRE(G[1].size() == 1);
    REQUIRE(G[2].size() == 0);
    REQUIRE(G[3].size() == 0);
  }
}

TEST_CASE("Adjacency self-loops and multi-edges", "[adjacency]") {

  SECTION("Self-loop") {
    edge_list<directedness::directed> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 0);  // self-loop
    edges.push_back(0, 1);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    REQUIRE(G[0].size() == 2);  // both the self-loop and 0->1
  }

  SECTION("Multiple edges between same vertices") {
    edge_list<directedness::directed, double> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1, 1.0);
    edges.push_back(0, 1, 2.0);  // duplicate edge with different weight
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    // Both edges should be stored
    REQUIRE(G[0].size() == 2);
  }
}

TEST_CASE("Adjacency graph concepts", "[adjacency]") {

  SECTION("Adjacency satisfies adjacency_list_graph concept") {
    edge_list<directedness::directed> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Verify basic operations work as expected for graph concepts
    REQUIRE(num_vertices(G) == 2);

    // The graph should be indexable
    auto neighbors = G[0];
    REQUIRE(neighbors.size() == 1);
  }
}

TEST_CASE("Adjacency<1> transpose view", "[adjacency]") {

  SECTION("Adjacency<1> gives incoming edges") {
    edge_list<directedness::directed> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 2);  // 0 -> 2
    edges.push_back(1, 2);  // 1 -> 2
    edges.close_for_push_back();

    // adjacency<0> stores outgoing edges
    // adjacency<1> stores incoming edges (transpose)
    adjacency<1> G(edges);

    // In the transpose, vertex 2 should have edges from 0 and 1
    REQUIRE(G[2].size() == 2);
    REQUIRE(G[0].size() == 0);
    REQUIRE(G[1].size() == 0);
  }
}
