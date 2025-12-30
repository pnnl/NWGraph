/**
 * @file vertex_range_test.cpp
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
#include <atomic>
#include <numeric>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

// Helper to create a small test graph
auto create_small_graph() {
  edge_list<directedness::directed> edges(5);
  edges.open_for_push_back();
  edges.push_back(0, 1);
  edges.push_back(0, 2);
  edges.push_back(1, 2);
  edges.push_back(1, 3);
  edges.push_back(2, 4);
  edges.push_back(3, 4);
  edges.close_for_push_back();
  return edges;
}

TEST_CASE("Vertex range basic operations", "[vertex-range]") {

  SECTION("Iterate over all vertices using vertex_range") {
    auto edges = create_small_graph();
    adjacency<0> G(edges);

    std::vector<size_t> vertices;
    for (auto v : vertex_range<adjacency<0>>(G.size())) {
      vertices.push_back(v);
    }

    REQUIRE(vertices.size() == 5);
    // Should be in order 0, 1, 2, 3, 4
    for (size_t i = 0; i < 5; ++i) {
      REQUIRE(vertices[i] == i);
    }
  }

  SECTION("Single vertex") {
    std::vector<size_t> vertices;
    for (auto v : vertex_range<adjacency<0>>(1)) {
      vertices.push_back(v);
    }

    REQUIRE(vertices.size() == 1);
    REQUIRE(vertices[0] == 0);
  }
}

TEST_CASE("Edge range basic operations", "[edge-range]") {

  SECTION("Iterate over all edges with weighted graph") {
    edge_list<directedness::directed, double> edges(5);
    edges.open_for_push_back();
    edges.push_back(0, 1, 1.0);
    edges.push_back(0, 2, 2.0);
    edges.push_back(1, 2, 3.0);
    edges.push_back(1, 3, 4.0);
    edges.push_back(2, 4, 5.0);
    edges.push_back(3, 4, 6.0);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    size_t edge_count = 0;
    for (auto&& [u, v, w] : make_edge_range<0>(G)) {
      edge_count++;
      // Verify u < num_vertices
      REQUIRE(u < 5);
      REQUIRE(v < 5);
      (void)w;
    }

    REQUIRE(edge_count == 6);
  }

  SECTION("Edge range with TBB parallel_for") {
    edge_list<directedness::directed, double> edges(5);
    edges.open_for_push_back();
    edges.push_back(0, 1, 1.0);
    edges.push_back(0, 2, 2.0);
    edges.push_back(1, 2, 3.0);
    edges.push_back(1, 3, 4.0);
    edges.push_back(2, 4, 5.0);
    edges.push_back(3, 4, 6.0);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    std::atomic<size_t> edge_count{0};
    tbb::parallel_for(edge_range<decltype(G), 0>(G), [&](auto&& subrange) {
      for (auto&& [u, v, w] : subrange) {
        edge_count++;
        (void)u;
        (void)v;
        (void)w;
      }
    });

    REQUIRE(edge_count == 6);
  }
}

TEST_CASE("Neighbor iteration operations", "[neighbor-range]") {

  SECTION("Iterate neighbors of specific vertex") {
    auto edges = create_small_graph();
    adjacency<0> G(edges);

    // Get neighbors of vertex 0
    std::vector<size_t> neighbors_of_0;
    for (auto&& [v] : G[0]) {
      neighbors_of_0.push_back(v);
    }

    REQUIRE(neighbors_of_0.size() == 2);
    // Should be 1 and 2 (in some order)
    std::sort(neighbors_of_0.begin(), neighbors_of_0.end());
    REQUIRE(neighbors_of_0[0] == 1);
    REQUIRE(neighbors_of_0[1] == 2);
  }

  SECTION("Neighbors of sink vertex") {
    auto edges = create_small_graph();
    adjacency<0> G(edges);

    // Vertex 4 is a sink (no outgoing edges)
    std::vector<size_t> neighbors_of_4;
    for (auto&& [v] : G[4]) {
      neighbors_of_4.push_back(v);
    }

    REQUIRE(neighbors_of_4.size() == 0);
  }
}

TEST_CASE("Range adaptors with algorithms", "[range-algorithms]") {

  SECTION("Count edges using ranges") {
    auto edges = create_small_graph();
    adjacency<0> G(edges);

    size_t total_edges = 0;
    for (size_t u = 0; u < G.size(); ++u) {
      total_edges += G[u].size();
    }

    REQUIRE(total_edges == 6);
  }

  SECTION("Find vertex with maximum out-degree") {
    auto edges = create_small_graph();
    adjacency<0> G(edges);

    size_t max_degree = 0;
    size_t max_vertex = 0;

    for (size_t v = 0; v < G.size(); ++v) {
      if (G[v].size() > max_degree) {
        max_degree = G[v].size();
        max_vertex = v;
      }
    }

    // Vertices 0 and 1 both have degree 2
    REQUIRE(max_degree == 2);
    REQUIRE((max_vertex == 0 || max_vertex == 1));
  }

  SECTION("Compute adjacency list representation") {
    auto edges = create_small_graph();
    adjacency<0> G(edges);

    std::vector<std::vector<size_t>> adj_list(5);

    for (size_t u = 0; u < G.size(); ++u) {
      for (auto&& [v] : G[u]) {
        adj_list[u].push_back(v);
      }
    }

    // Verify adjacency list matches graph
    std::sort(adj_list[0].begin(), adj_list[0].end());
    REQUIRE(adj_list[0] == std::vector<size_t>{1, 2});

    std::sort(adj_list[1].begin(), adj_list[1].end());
    REQUIRE(adj_list[1] == std::vector<size_t>{2, 3});
  }
}

TEST_CASE("Weighted graph iteration", "[weighted-graph]") {

  SECTION("Iterate weighted edges") {
    edge_list<directedness::directed, double> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 1, 1.5);
    edges.push_back(0, 2, 2.5);
    edges.push_back(1, 2, 3.5);
    edges.close_for_push_back();

    adjacency<0, double> G(edges);

    double total_weight = 0.0;
    for (size_t u = 0; u < G.size(); ++u) {
      for (auto&& [v, w] : G[u]) {
        total_weight += w;
      }
    }

    REQUIRE(total_weight == Approx(7.5));
  }
}
